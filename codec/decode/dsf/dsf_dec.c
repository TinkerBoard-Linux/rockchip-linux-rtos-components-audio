#include "dsf_dec.h"

#include "dsd2pcm_conv.h"

int dsf_open(struct dsf *dsf)
{
    struct chunk ck;
    struct dsd_buf ck_data = {NULL, 0, 0};

    if (NULL == resize_dsd_buf(&ck_data, 1024))
        return -1;

    /* Read DSD tag */
    dsf->read(dsf, (char *)&ck, sizeof(ck));
    if (!chunk_id_is((char *)ck.id, "DSD "))
        return -2;
    if (NULL == resize_dsd_buf(&ck_data, data_le32(ck.size) - sizeof(ck)))
        return -1;
    if (dsf->read(dsf, (char *)ck_data.buf, ck_data.size) != ck_data.size)
        return -2;
    dsf->filesize = data_le32(ck_data.buf);
    dsf->id3_pos  = data_le32(ck_data.buf + 8);

    /* Read fmt tag */
    dsf->read(dsf, (char *)&ck, sizeof(ck));
    if (!chunk_id_is((char *)ck.id, "fmt "))
        return -2;
    if (NULL == resize_dsd_buf(&ck_data, data_le32(ck.size) - sizeof(ck)))
        return -1;
    if (dsf->read(dsf, (char *)ck_data.buf, ck_data.size) != ck_data.size)
        return -2;
    dsf->format_version = data_le32(ck_data.buf);
    dsf->format_id      = data_le32(ck_data.buf + 4);
    dsf->channel_type   = data_le32(ck_data.buf + 8);
    dsf->channels       = data_le32(ck_data.buf + 12);
    dsf->frequency      = data_le32(ck_data.buf + 16);
    dsf->bits           = data_le32(ck_data.buf + 20);
    dsf->samples        = data_le32(ck_data.buf + 24);
    dsf->block_size     = data_le32(ck_data.buf + 32);
    dsf->reserved       = data_le32(ck_data.buf + 36);

    /* Read fmt tag */
    dsf->read(dsf, (char *)&ck, sizeof(ck));
    if (!chunk_id_is((char *)ck.id, "data"))
        return -2;
    dsf->sample_bytes = data_le32(ck.size) - sizeof(ck);

    dsf->frames = dsf->sample_bytes / (dsf->block_size * dsf->channels);

    resize_dsd_buf(&ck_data, 0);

    dsf->out_ch = dsf->channels;

    switch (dsf->frequency)
    {
    case 2822400:
        dsf->type = DSD64;
        dsf->out_rate = 44100;
        break;
    case 5644800:
        dsf->type = DSD128;
        dsf->out_rate = 176400;
        break;
    case 11289600:
        dsf->type = DSD256;
        dsf->out_rate = 88200;
        break;
    default:
        dsf->type = DSD64;
        dsf->out_rate = 44100;
        break;
    }

    return RK_AUDIO_SUCCESS;
}

int dsf_process(struct dsf *dsf)
{
    void* dsd2pcm;
    uint8_t *out;
    uint8_t *in;
    uint32_t out_size;
    uint32_t frame_size;
    int read_ret, write_ret, ret = RK_AUDIO_FAILURE;

    frame_size = dsf->out_ch * dsf->block_size;
    out_size = frame_size / (dsf->frequency / dsf->out_rate >> 3) * sizeof(uint32_t);
    out = audio_malloc(out_size);
    if (!out)
    {
        RK_AUDIO_LOG_E("malloc %ld failed", out_size);
        goto out_err;
    }
    in = audio_malloc(frame_size);
    if (!in)
    {
        RK_AUDIO_LOG_E("malloc %ld failed", frame_size);
        goto in_err;
    }
    dsd2pcm = dsd2pcm_converter_init(dsf->type, dsf->channels, dsf->frequency, dsf->out_rate);
    if (!dsd2pcm)
    {
        RK_AUDIO_LOG_E("create dsd2pcm failed");
        goto dsd_err;
    }
    RK_AUDIO_LOG_V("%ld [%ld] => %ld [%d]\n", frame_size, dsf->frequency, out_size, dsf->out_rate);
    for (int i = 0; i < dsf->frames; i++)
    {
        read_ret = dsf->read(dsf, (char *)in, frame_size);
        if (read_ret != frame_size)
        {
            if (read_ret == -1)
                ret = RK_AUDIO_FAILURE;
            else
                ret = RK_AUDIO_SUCCESS;
            break;
        }

        /* dsd to pcm */
        int len = dsd2pcm_converter_convert(dsd2pcm, in, frame_size, out, 32, 1);
        write_ret = dsf->write(dsf, (char *)out, len * sizeof(uint32_t));
        if (write_ret != len * sizeof(uint32_t))
        {
            ret = RK_AUDIO_FAILURE;
            break;
        }
    }
    dsd2pcm_converter_deinit(dsd2pcm);
dsd_err:
    audio_free(in);
in_err:
    audio_free(out);
out_err:

    return ret;
}

