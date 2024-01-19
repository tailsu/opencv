/*
 * jcapistd.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains application interface code for the compression half
 * of the JPEG library.  These are the "standard" API routines that are
 * used in the normal full-compression case.  They are not used by a
 * transcoding-only application.  Note that if an application links in
 * jpeg_start_compress, it will end up linking in the entire compressor.
 * We thus must separate this file from jcapimin.c to avoid linking the
 * whole compression library into a transcoder.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


/*
 * Compression initialization.
 * Before calling this, all parameters and a data destination must be set up.
 *
 * We require a write_all_tables parameter as a failsafe check when writing
 * multiple datastreams from the same compression object.  Since prior runs
 * will have left all the tables marked sent_table=TRUE, a subsequent run
 * would emit an abbreviated stream (no tables) by default.  This may be what
 * is wanted, but for safety's sake it should not be the default behavior:
 * programmers should have to make a deliberate choice to emit abbreviated
 * images.  Therefore the documentation and examples should encourage people
 * to pass write_all_tables=TRUE; then it will take active thought to do the
 * wrong thing.
 */

GLOBAL(void)
jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables)
{
  if (cinfo->global_state != CSTATE_START)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);

  if (write_all_tables)
    jpeg_suppress_tables(cinfo, FALSE); /* mark all tables to be written */

  /* (Re)initialize error mgr and destination modules */
  (*cinfo->err->reset_error_mgr) ((j_common_ptr)cinfo);
  (*cinfo->dest->init_destination) (cinfo);
  /* Perform master selection of active modules */
  jinit_compress_master(cinfo);
  /* Set up for the first pass */
  (*cinfo->master->prepare_for_pass) (cinfo);
  /* Ready for application to drive first pass through jpeg_write_scanlines
   * or jpeg_write_raw_data.
   */
  cinfo->next_scanline = 0;
  cinfo->batch = NULL;
  cinfo->global_state = (cinfo->raw_data_in ? CSTATE_RAW_OK : CSTATE_SCANNING);
}

LOCAL(void) clone_compress_struct(j_compress_ptr dst, j_compress_ptr src) {
  memset(dst, 0, sizeof(struct jpeg_compress_struct));

  dst->err = src->err;
  dst->mem = src->mem;
  // dst->progress : no progress allowed in clones
  dst->client_data = src->client_data;
  dst->is_decompressor = src->is_decompressor;
  dst->global_state = src->global_state; // clone starts in same state as original

  // dst->dest : clones will receive their own destination
  dst->image_width = src->image_width;
  dst->image_height = src->image_height;
  dst->input_components = src->input_components;
  dst->in_color_space = src->in_color_space;
  dst->input_gamma = src->input_gamma;

#if JPEG_LIB_VERSION >= 70
  dst->scale_num = src->scale_num;
  dst->scale_denom = src->scale_denom;
  dst->jpeg_width = src->jpeg_width;
  dst->jpeg_height = src->jpeg_height;
#endif

  dst->data_precision = src->data_precision;
  dst->num_components = src->num_components;
  dst->jpeg_color_space = src->jpeg_color_space;
  dst->comp_info = src->comp_info; // component info is read-only
  for (int i = 0; i < NUM_QUANT_TBLS; ++i) {
    dst->quant_tbl_ptrs[i] = src->quant_tbl_ptrs[i]; // quant tables are read-only
#if JPEG_LIB_VERSION >= 70
    dst->q_scale_factor[i] = src->q_scale_factor[i];
#endif
  }
  for (int i = 0; i < NUM_HUFF_TBLS; ++i) {
    dst->dc_huff_tbl_ptrs[i] = src->dc_huff_tbl_ptrs[i]; // huff tables are read-only
    dst->ac_huff_tbl_ptrs[i] = src->ac_huff_tbl_ptrs[i]; // huff tables are read-only
  }
  // arith: skip
  // scans: skip
  // raw_data_in: is false
  // arith_code: is false
  // optimize_coding: is false
  // CCIR601_sampling: is false, not implemented
#if JPEG_LIB_VERSION >= 70
  // dst->do_fancy_downsampling : libjpeg-turbo doesn't implement this
#endif
  dst->smoothing_factor = src->smoothing_factor;
  dst->dct_method = src->dct_method;

  // dst->restart_interval = src->restart_interval;
  // restart_interval: must be 0
  // restart_in_rows: must be 0

  dst->write_JFIF_header = src->write_JFIF_header;
  dst->JFIF_major_version = src->JFIF_major_version;
  dst->JFIF_minor_version = src->JFIF_minor_version;
  dst->density_unit = src->density_unit;
  dst->X_density = src->X_density;
  dst->Y_density = src->Y_density;
  dst->write_Adobe_marker = src->write_Adobe_marker;

  // next_scanline: managed separately

  // progressive_mode: is false

  dst->max_h_samp_factor = src->max_h_samp_factor;
  dst->max_v_samp_factor = src->max_v_samp_factor;
#if JPEG_LIB_VERSION >= 70
  dst->min_DCT_h_scaled_size = src->min_DCT_h_scaled_size;
  dst->min_DCT_v_scaled_size = src->min_DCT_v_scaled_size;
#endif

  dst->total_iMCU_rows = src->total_iMCU_rows;
  dst->comps_in_scan = src->comps_in_scan;
  for (int i = 0; i < MAX_COMPS_IN_SCAN; ++i) {
    dst->cur_comp_info[i] = src->cur_comp_info[i]; // component info is read-only
  }
  dst->MCUs_per_row = src->MCUs_per_row;
  dst->MCU_rows_in_scan = src->MCU_rows_in_scan;
  dst->blocks_in_MCU = src->blocks_in_MCU;
  for (int i = 0; i < C_MAX_BLOCKS_IN_MCU; ++i) {
    dst->MCU_membership[i] = src->MCU_membership[i];
  }
  // progressive is false, but copy anyway
  dst->Ss = src->Ss;
  dst->Se = src->Se;
  dst->Ah = src->Ah;
  dst->Al = src->Al;

#if JPEG_LIB_VERSION >= 80
  dst->block_size = src->block_size;
  dst->natural_order = src->natural_order;
  dst->lim_Se = src->lim_Se;
#endif

  // begin clone child structures
  // same order as in jinit_compress_master
  // master: not available to clones
  
  dst->batch = src->batch; // jinits can check this to see if batching is enabled
  dst->cconvert = src->cconvert; // color conversion is read-only
  dst->downsample = src->downsample; // downsampling is read-only

  jinit_c_prep_controller(dst, FALSE); // requires dst->downsample
  jinit_forward_dct(dst);
  jinit_huff_encoder(dst);
  jinit_c_coef_controller(dst, FALSE);
  jinit_c_main_controller(dst, FALSE);

  // marker: not available to clones
  // script_space: progressive is false
  // script_space_size: progressive is false
}

LOCAL(void) compress_clone_start_pass(j_compress_ptr dst) {
  dst->prep->start_pass(dst, JBUF_PASS_THRU); //simple initialization
  dst->fdct->start_pass(dst); //simple initialization
  dst->entropy->start_pass(dst, FALSE); //simple initialization
  dst->coef->start_pass(dst, JBUF_PASS_THRU); //simple initialization
  dst->main->start_pass(dst, JBUF_PASS_THRU); //simple initialization
}

#define C_JPEG_OPTIMIZE_BATCHING 1

LOCAL(void) init_compress_batching(j_compress_ptr cinfo) {
  cinfo->enable_batching = FALSE;

  if (!jpeg_get_parallel_impl()) {
    // parallel implementation not available, no point in batching
    return;
  }

  if (cinfo->num_scans != 1 || cinfo->optimize_coding) {
    // not supported
    return;
  }

  if (cinfo->restart_interval != 0 && cinfo->restart_interval % cinfo->MCUs_per_row != 0) {
    WARNMS(cinfo, JWRN_RSTINTERVAL_WHOLE_ROW);
    return;
  }

  int num_batches = jdiv_round_up(cinfo->MCUs_per_row * cinfo->MCU_rows_in_scan, cinfo->restart_interval);

  if (num_batches > cinfo->MCU_rows_in_scan) {
    WARNMS(cinfo, JWRN_RSTINTERVAL_NOT_ENOUGH_ROWS);
    return;
  }

  if (num_batches > C_JPEG_MAX_BATCHES) {
    WARNMS(cinfo, JWRN_RSTINTERVAL_TOO_MANY_BATCHES);
    return;
  }

  if (num_batches == 1 && C_JPEG_OPTIMIZE_BATCHING) {
    // no point in enabling batching for a single batch
    return;
  }
  if (cinfo->image_width * cinfo->image_height < 256 * 256 || cinfo->image_height < cinfo->max_v_samp_factor * DCTSIZE * 2) {
    // avoid batching overhead for small images
    if (C_JPEG_OPTIMIZE_BATCHING) {
      return;
    }
  }

  cinfo->enable_batching = TRUE;
  cinfo->batch = (struct jpeg_batch_writer *)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(struct jpeg_batch_writer));
  memset(cinfo->batch, 0, sizeof(struct jpeg_batch_writer));
                            
  cinfo->batch->num_batches = num_batches;
  cinfo->batch->scanlines = (JSAMPARRAY)
    (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                sizeof(JSAMPROW) * cinfo->image_height);

  int mcu_rows_per_batch = jdiv_round_up(cinfo->MCU_rows_in_scan, num_batches);
  int scanlines_per_mcu = cinfo->max_v_samp_factor * DCTSIZE;
  int step = mcu_rows_per_batch * scanlines_per_mcu;

  // printf("batching %d batches for height %d in_color_space %d v_samp %d h_samp %d \n", num_batches, cinfo->image_height, cinfo->in_color_space, cinfo->max_v_samp_factor, cinfo->max_h_samp_factor);

  for (int i = 0; i < num_batches; ++i) {
    cinfo->batch->cinfos[i] = (j_compress_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                  sizeof(struct jpeg_compress_struct));

    j_compress_ptr ci = cinfo->batch->cinfos[i];
    clone_compress_struct(ci, cinfo);

    jpeg_mem_dest(ci, &cinfo->batch->buffers[i].data, &cinfo->batch->buffers[i].size);

    ci->start_row_in_batch = i * step;
    ci->rows_in_batch = i != num_batches - 1 ? step : cinfo->image_height - ci->start_row_in_batch;

    compress_clone_start_pass(ci);

    // opencv sets an error handler with setjmp. setjmp cannot jump across threads,
    // so we can't allow errors to be reported from the workers.
    // on the other hand we don't expect any errors from here on, so we can just ignore them.
    ci->err = NULL;
    
    // allocations aren't thread-safe, no more allocations allowed in clones.
    ci->mem = NULL;
  }
}

LOCAL(void) flush_batches(j_compress_ptr cinfo) {
  for (int i = 0; i < cinfo->batch->num_batches; ++i) {
    j_compress_ptr ci = cinfo->batch->cinfos[i];
    ci->dest->term_destination(ci);
  }

  // write the lengths of the first N-1 compressed streams in an APP9 marker for fast decoding
  int num_data_length_components = 2 /*header*/ + cinfo->batch->num_batches - 1;
  uint32_t* data_length_marker = cinfo->mem->alloc_small((j_common_ptr)cinfo, JPOOL_IMAGE, sizeof(uint32_t) * num_data_length_components);
  data_length_marker[0] = C_JPEG_DATA_LENGTHS_MAGIC0;
  data_length_marker[1] = C_JPEG_DATA_LENGTHS_MAGIC1;
  for (int i = 0; i < cinfo->batch->num_batches - 1; ++i) {
    data_length_marker[2 + i] = cinfo->batch->buffers[i].size;
  }

  cinfo->next_scanline = 0; // to placate the state check in jpeg_write_marker
  jpeg_write_marker(cinfo, C_JPEG_DATA_LENGTHS_MARKER, (JOCTET*)data_length_marker, sizeof(uint32_t) * num_data_length_components);
  cinfo->next_scanline = cinfo->image_height;

  (*cinfo->master->pass_startup) (cinfo); // writes SOF and SOS

  for (int i = 0; i < cinfo->batch->num_batches; ++i) {
    JOCTET* data = cinfo->batch->buffers[i].data;
    for (size_t remaining = cinfo->batch->buffers[i].size; remaining != 0; ) {
      size_t tocopy = MIN(remaining, cinfo->dest->free_in_buffer);
      memcpy(cinfo->dest->next_output_byte, data, tocopy);
      remaining -= tocopy;
      data += tocopy;
      cinfo->dest->next_output_byte += tocopy;
      cinfo->dest->free_in_buffer -= tocopy;
      if (cinfo->dest->free_in_buffer == 0) {
        cinfo->dest->empty_output_buffer(cinfo);
      }
    }
    free(cinfo->batch->buffers[i].data);

    if (i != cinfo->batch->num_batches - 1) {
      cinfo->marker->write_marker_byte(cinfo, 0xFF);
      cinfo->marker->write_marker_byte(cinfo, JPEG_RST0 + (i & 7));
    }
  }

  cinfo->batch = NULL;
}

LOCAL(void) batch_compress_worker(void* context, int batch_index)
{
  j_compress_ptr cinfo = (j_compress_ptr)context;
  j_compress_ptr ci = cinfo->batch->cinfos[batch_index];
  JDIMENSION row_ctr = 0;
  ci->main->process_data(ci, &cinfo->batch->scanlines[ci->start_row_in_batch], &row_ctr, ci->rows_in_batch);
  ci->entropy->finish_pass(ci);
}

/*
 * Write some scanlines of data to the JPEG compressor.
 *
 * The return value will be the number of lines actually written.
 * This should be less than the supplied num_lines only in case that
 * the data destination module has requested suspension of the compressor,
 * or if more than image_height scanlines are passed in.
 *
 * Note: we warn about excess calls to jpeg_write_scanlines() since
 * this likely signals an application programmer error.  However,
 * excess scanlines passed in the last valid call are *silently* ignored,
 * so that the application need not adjust num_lines for end-of-image
 * when using a multiple-scanline buffer.
 */

GLOBAL(JDIMENSION)
jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines,
                     JDIMENSION num_lines)
{
  JDIMENSION rows_left;

  if (cinfo->global_state != CSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline >= cinfo->image_height)
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);

  /* Call progress monitor hook if present */
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long)cinfo->next_scanline;
    cinfo->progress->pass_limit = (long)cinfo->image_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr)cinfo);
  }

  /* Ignore any extra scanlines at bottom of image. */
  rows_left = cinfo->image_height - cinfo->next_scanline;
  if (num_lines > rows_left)
    num_lines = rows_left;

  if (cinfo->enable_batching && !cinfo->batch) {
    init_compress_batching(cinfo);
  }

  if (cinfo->batch) {
    for (int i = 0; i < num_lines; ++i) {
      cinfo->batch->scanlines[cinfo->next_scanline++] = scanlines[i];
    }

    if (cinfo->next_scanline == cinfo->image_height) {
      struct jpeg_parallel_impl* parallel = jpeg_get_parallel_impl();

      parallel->apply(batch_compress_worker, cinfo->batch->num_batches, cinfo);

      flush_batches(cinfo);
    }
  } else {
    /* Give master control module another chance if this is first call to
    * jpeg_write_scanlines.  This lets output of the frame/scan headers be
    * delayed so that application can write COM, etc, markers between
    * jpeg_start_compress and jpeg_write_scanlines.
    */
    if (cinfo->master->call_pass_startup)
      (*cinfo->master->pass_startup) (cinfo);

    JDIMENSION row_ctr = 0;
    cinfo->main->process_data(cinfo, scanlines, &row_ctr, num_lines);
    cinfo->next_scanline += row_ctr;
    return row_ctr;
  }

  return num_lines;
}


/*
 * Alternate entry point to write raw data.
 * Processes exactly one iMCU row per call, unless suspended.
 */

GLOBAL(JDIMENSION)
jpeg_write_raw_data(j_compress_ptr cinfo, JSAMPIMAGE data,
                    JDIMENSION num_lines)
{
  JDIMENSION lines_per_iMCU_row;

  if (cinfo->global_state != CSTATE_RAW_OK)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->next_scanline >= cinfo->image_height) {
    WARNMS(cinfo, JWRN_TOO_MUCH_DATA);
    return 0;
  }

  /* Call progress monitor hook if present */
  if (cinfo->progress != NULL) {
    cinfo->progress->pass_counter = (long)cinfo->next_scanline;
    cinfo->progress->pass_limit = (long)cinfo->image_height;
    (*cinfo->progress->progress_monitor) ((j_common_ptr)cinfo);
  }

  /* Give master control module another chance if this is first call to
   * jpeg_write_raw_data.  This lets output of the frame/scan headers be
   * delayed so that application can write COM, etc, markers between
   * jpeg_start_compress and jpeg_write_raw_data.
   */
  if (cinfo->master->call_pass_startup)
    (*cinfo->master->pass_startup) (cinfo);

  /* Verify that at least one iMCU row has been passed. */
  lines_per_iMCU_row = cinfo->max_v_samp_factor * DCTSIZE;
  if (num_lines < lines_per_iMCU_row)
    ERREXIT(cinfo, JERR_BUFFER_SIZE);

  /* Directly compress the row. */
  if (!(*cinfo->coef->compress_data) (cinfo, data)) {
    /* If compressor did not consume the whole row, suspend processing. */
    return 0;
  }

  /* OK, we processed one iMCU row. */
  cinfo->next_scanline += lines_per_iMCU_row;
  return lines_per_iMCU_row;
}
