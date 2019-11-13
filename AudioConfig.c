/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
static struct audio_server_data_share g_ABCoreDat =
{
    0
};
#if defined(OS_IS_FREERTOS)
audio_player_thread_handle audio_thread_create(char *name, uint32_t StackDeep, uint32_t Priority, audio_thread_cfg_t *cfg)
{
    RK_AUDIO_LOG_D("audio_thread_create.");
    return rktm_create_task((pTaskFunType)cfg->run, NULL, NULL, name, StackDeep, Priority, (void *)cfg->args);
}

void audio_thread_exit(audio_player_thread_handle self)
{
    rktm_delete_task((HTC)self);
}

audio_player_semaphore_handle audio_semaphore_create(void)
{
    return rkos_semaphore_create(1, 0);
}

int audio_semaphore_take(audio_player_semaphore_handle self)
{
    return rkos_semaphore_take(self, -1);
}

int audio_semaphore_give(audio_player_semaphore_handle self)
{
    return rkos_semaphore_give(self);
}

void audio_semaphore_destroy(audio_player_semaphore_handle self)
{
    rkos_semaphore_delete(self);
}

audio_player_mutex_handle audio_mutex_create(void)
{
    return rkos_mutex_create();
}

int audio_mutex_lock(audio_player_mutex_handle self)
{
    return rkos_semaphore_take(self, MAX_DELAY);
}

int audio_mutex_unlock(audio_player_mutex_handle self)
{
    return rkos_semaphore_give(self);
}

void audio_mutex_destroy(audio_player_mutex_handle self)
{
    rkos_semaphore_delete(self);
}

void *audio_timer_create(char *name, uint32_t period, uint32_t reload, void *param, void (*timer_callback)(void *))
{
    return rkos_create_timer(period, reload, param, timer_callback);
}

int audio_timer_control(void *timer, uint32_t new_period, uint32_t over_time)
{
    return rkos_mod_timer(timer, new_period, over_time);
}

int audio_timer_start(void *timer)
{
    return rkos_start_timer(timer);
}

int audio_timer_stop(void *timer)
{
    return rkos_stop_timer(timer);
}

int audio_timer_delete(void *timer)
{
    return rkos_delete_timer(timer);
}

void *audio_malloc(size_t size)
{
    return rkos_memory_malloc(size);
}

void audio_free(void *ptr)
{
    rkos_memory_free(ptr);
}

void audio_free_uncache(void *ptr)
{
    rkos_memory_free_align(ptr);
}

void *audio_calloc(size_t nmemb, size_t size)
{
    size_t SizeLen = nmemb * size;
    return rkos_memory_malloc(SizeLen);
}

void *audio_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void *audio_malloc_uncache(size_t size)
{
    return rkos_memory_malloc_align(size, 32);
}

void *audio_device_open(const int dev_id, int flag)
{
    DEVICE_CLASS *audio_dev;
    audio_dev = (DEVICE_CLASS *)rkdev_open(DEV_CLASS_AUDIO, dev_id, 0x00);
    if (audio_dev == NULL)
    {
        RK_AUDIO_LOG_E("can not open audio card %d.\n", dev_id);
        return NULL;
    }

    return audio_dev;
}

int audio_device_control(void *dev, uint32_t cmd, void *arg)
{
    return rkdev_control(dev, cmd, arg);
}

int audio_device_close(void *dev)
{
    return rkdev_close(dev);
}

unsigned long audio_device_write(void *dev, char *data, unsigned long frames)
{
    return rkdev_write(dev, 0, data, frames);
}

unsigned long audio_device_read(void *dev, char *data, unsigned long frames)
{
    return rkdev_read(dev, 0, data, frames);
}

void audio_sleep(uint32_t ms)
{
    rkos_sleep(ms);
}

#else
audio_player_thread_handle audio_thread_create(char *name, uint32_t StackDeep, uint32_t Priority, audio_thread_cfg_t *cfg)
{
    RK_AUDIO_LOG_D("audio_thread_create.");
    rt_thread_t tid;
    tid = rt_thread_create(name, cfg->run, cfg->args, StackDeep, Priority, 10);
    if (tid)
        rt_thread_startup(tid);
    return (audio_player_thread_handle)tid;
}

void audio_thread_exit(audio_player_thread_handle self)
{
    rt_thread_delete((rt_thread_t)self);
}

audio_player_semaphore_handle audio_semaphore_create(void)
{
    return (audio_player_semaphore_handle)rt_sem_create("os_sem", 1, RT_IPC_FLAG_PRIO);
}

int audio_semaphore_take(audio_player_semaphore_handle self)
{
    return rt_sem_take(self, RT_WAITING_FOREVER);
}

int audio_semaphore_give(audio_player_semaphore_handle self)
{
    return rt_sem_release(self);
}

void audio_semaphore_destroy(audio_player_semaphore_handle self)
{
    rt_sem_delete(self);
}

audio_player_mutex_handle audio_mutex_create(void)
{
    return (audio_player_mutex_handle)rt_mutex_create("os_mutex", RT_IPC_FLAG_PRIO);
}

int audio_mutex_lock(audio_player_mutex_handle self)
{
    return rt_mutex_take((rt_mutex_t)self, RT_WAITING_FOREVER);
}

int audio_mutex_unlock(audio_player_mutex_handle self)
{
    return rt_mutex_release((rt_mutex_t)self);
}

void audio_mutex_destroy(audio_player_mutex_handle self)
{
    rt_mutex_delete((rt_mutex_t)self);
}

void *audio_timer_create(char *name, uint32_t period, uint32_t reload, void *param, void (*timer_callback)(void *))
{
    return rt_timer_create(name, timer_callback, param, period, reload);
}

int audio_timer_control(void *timer, uint32_t new_period, uint32_t over_time)
{
    return rt_timer_control(timer, RT_TIMER_CTRL_SET_TIME, &new_period);
}

int audio_timer_start(void *timer)
{
    return rt_timer_start(timer);
}

int audio_timer_stop(void *timer)
{
    return rt_timer_stop(timer);
}

int audio_timer_delete(void *timer)
{
    return rt_timer_delete(timer);
}

void *audio_malloc(size_t size)
{
    return rt_malloc(size);
}

void audio_free(void *ptr)
{
    rt_free(ptr);
}

void audio_free_uncache(void *ptr)
{
    rt_free_uncache(ptr);
}

void *audio_calloc(size_t nmemb, size_t size)
{
    return rt_calloc(nmemb, size);
}

void *audio_realloc(void *ptr, size_t size)
{
    return rt_realloc(ptr, size);
}

void *audio_malloc_uncache(size_t size)
{
    return rt_malloc_uncache(size);
}

const char *sound_card[5] =
{
    "adcc",
    "pdmc",
    "es8388c",
    "es8388p",
    "audpwmp",
}

void *audio_device_open(const int dev_id, int flag)
{
    struct rt_device *audio_dev;
    int devid;
    if (flag == AUDIO_FLAG_WRONLY)
        devid = dev_id + 3;
    else
        devid = dev_id;
    audio_dev = rt_device_find(sound_card[devid]);
    if (audio_dev == RT_NULL)
    {
        RK_AUDIO_LOG_E("can not find audio device %s.\n", dev_name);
        return NULL;
    }
    int ret = rt_device_open(audio_dev, flag);
    if (ret)
    {
        RK_AUDIO_LOG_E("can not open audio device %s %d.\n", dev_name, ret);
        return NULL;
    }

    return audio_dev;
}

int audio_device_control(void *dev, uint32_t cmd, void *arg)
{
    return rt_device_control((rt_device_t)dev, cmd, arg);
}

int audio_device_close(void *dev)
{
    return rt_device_close((rt_device_t)dev);
}

unsigned long audio_device_write(void *dev, char *data, unsigned long frames)
{
    return rt_device_write(dev, 0, data, frames);
}

unsigned long audio_device_read(void *dev, char *data, unsigned long frames)
{
    return rt_device_read(dev, 0, data, frames);
}

void audio_sleep(uint32_t ms)
{
    rt_thread_mdelay(ms);
}
#endif

int audio_fopen(char *path, char *mode)
{
#if defined(OS_IS_FREERTOS)
    void *FileFd = NULL;

    FileFd = rkos_file_open(path, mode);
    RK_ASSERT((int)FileFd <= 0x7fffffff);
    if (FileFd == NULL)
    {
        return RK_AUDIO_FAILURE;
    }
    else
    {
        return (int)FileFd;
    }
#else
    int audio_file = RK_AUDIO_FAILURE;
    if (strstr(mode, "+"))
    {
        audio_file = open(path, O_RDWR);
        if (audio_file == RK_AUDIO_FAILURE)
        {
            RK_AUDIO_LOG_E("file open O_RDWR failed\n");
            return RK_AUDIO_FAILURE;
        }
    }
    else if (strstr(mode, "r"))
    {
        audio_file = open(path, O_RDONLY);
        if (audio_file == RK_AUDIO_FAILURE)
        {
            RK_AUDIO_LOG_E("file open O_RDONLY faile\n");
            return RK_AUDIO_FAILURE;
        }
    }
    else if (strstr(mode, "w"))
    {
        audio_file = open(path, O_WRONLY);
        if (audio_file == RK_AUDIO_FAILURE)
        {
            RK_AUDIO_LOG_E("file open O_WRONLY faile\n");
            return RK_AUDIO_FAILURE;
        }
    }
    return audio_file;
#endif
}

int audio_fread(void *buffer, size_t size, size_t count, int stream)
{
#if defined(OS_IS_FREERTOS)
    int ret = 0;
    if (stream == -1)
    {
        rk_printf("\naudio_file = %d", stream);
        return 0;
    }
    ret = rkos_file_read(buffer, size, count, (HDC)stream);
    if (ret <= 0)
        return 0;
    return ret;
#else
    int data_len = size * count;
    int ret = 0;

    if (data_len == 0)
    {
        RK_AUDIO_LOG_W("NO read data.");
        return 0;
    }
    ret = read(stream, (uint8_t *)buffer, data_len);
    if (ret == -1)
        return 0;

    return ret;
#endif
}

int audio_fwrite(const void *buffer, size_t size, size_t count, int stream)
{
#if defined(OS_IS_FREERTOS)
    return rkos_file_write(buffer, size, count, (HDC)stream);
#else
    int data_len = size * count;
    int ret = 0;
    if (data_len == 0)
    {
        RK_AUDIO_LOG_W(" NO write data.\n");
        return 0;
    }

    ret = write(stream, (uint8_t *)buffer, data_len);
    if (ret <= 0)
        return 0;

    return ret;
#endif
}

int audio_fclose(int fd)
{
#if defined(OS_IS_FREERTOS)
    if (rkos_file_close((HDC)fd) != RK_SUCCESS)
    {
        return RK_AUDIO_FAILURE;
    }
    else
    {
        return RK_AUDIO_SUCCESS;
    }
#else
    return close(fd);
#endif
}

size_t audio_ftell(int stream)
{
#if defined(OS_IS_FREERTOS)
    return rkos_file_get_size((HDC)stream);
#else
    struct stat file_stat;
    fstat(stream, &file_stat);
    return file_stat.st_size;
#endif
}

int audio_fseek(int stream, int32_t offset, uint32_t pos)
{
#if defined(OS_IS_FREERTOS)
    int ret = rkos_file_lseek((HDC)stream, offset, pos);
    if (ret != RK_AUDIO_SUCCESS)
    {
        return RK_AUDIO_FAILURE;
    }
    return ret;
#else
    int ret = lseek(stream, offset, pos);
    if (ret < 0)
    {
        return RK_AUDIO_FAILURE;
    }
    return ret;
#endif
}

struct audio_player_queue *audio_queue_create(size_t item_count, size_t item_size)
{
    struct audio_player_queue *queue = (struct audio_player_queue *)audio_calloc(1, sizeof(*queue));
    int i;
    queue->item_count = item_count;
    queue->item_size = item_size;
    queue->fill = 0;
    queue->read_pos = 0;
    queue->write_pos = 0;
    queue->state = 0;
    queue->item = &queue->item_temp;
    RK_AUDIO_LOG_D("sizeof(*queue) = %d.", sizeof(*queue));
    for (i = 0; i < item_count; i++)
    {
        queue->item[i] = audio_calloc(1, item_size);
    }
    queue->lock = audio_mutex_create();
    queue->read_sem = audio_semaphore_create();
    queue->write_sem = audio_semaphore_create();
    RK_AUDIO_LOG_D("item_count create =%x, %d.", queue, item_count);

    return queue;
}

int audio_queue_send(struct audio_player_queue *self, const void *data)
{
    while (1)
    {
        audio_mutex_lock(self->lock);
        RK_AUDIO_LOG_D("s:%x,self->fill:%d,self->item_count:%d\n", self, self->fill, self->item_count);
        if (self->state & AUDIO_QUEUE_STATE_STOPPED)
        {
            audio_mutex_unlock(self->lock);
            return RK_AUDIO_FAILURE;
        }
        if (self->fill != self->item_count)
        {
            break;
        }
        self->state |= AUDIO_QUEUE_STATE_WAIT_WRITABLE;
        audio_mutex_unlock(self->lock);
        audio_semaphore_take(self->write_sem);
    }
    RK_AUDIO_LOG_D("self->read_pos:%d\n", self->read_pos);
    memcpy(self->item[self->read_pos], data, self->item_size);
    self->fill++;
    self->read_pos++;
    self->read_pos = self->read_pos % self->item_count;
    if (self->state & AUDIO_QUEUE_STATE_WAIT_READABLE)
    {
        self->state &= (~AUDIO_QUEUE_STATE_WAIT_READABLE);
        audio_semaphore_give(self->read_sem);
    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

int audio_queue_send_font(struct audio_player_queue *self, const void *data)
{
    return RK_AUDIO_SUCCESS;
}

int audio_queue_receive(struct audio_player_queue *self, void *data)
{
    while (1)
    {
        audio_mutex_lock(self->lock);
        RK_AUDIO_LOG_D("r:%x: self->fill:%d,self->item_count = %d\n", self, self->fill, self->item_count);
        RK_AUDIO_LOG_D("os_queue_state = %x.", (int)self->state);
        if (self->state & AUDIO_QUEUE_STATE_STOPPED)
        {
            RK_AUDIO_LOG_W("AUDIO_QUEUE_STATE_STOPPED.");
            audio_mutex_unlock(self->lock);
            return RK_AUDIO_FAILURE;
        }
        if (self->fill)
        {
            break;
        }
        if (self->state & AUDIO_QUEUE_STATE_FINISHED)
        {
            self->state &= (~AUDIO_QUEUE_STATE_FINISHED);
            audio_mutex_unlock(self->lock);
            RK_AUDIO_LOG_W("AUDIO_QUEUE_STATE_FINISHED.");
            return RK_AUDIO_SUCCESS;
        }
        self->state |= AUDIO_QUEUE_STATE_WAIT_READABLE;
        RK_AUDIO_LOG_D("os_queue_state = %x", (int)self->state);
        audio_mutex_unlock(self->lock);
        audio_semaphore_take(self->read_sem);
    }
    RK_AUDIO_LOG_D("self->write_pos:%d, size = %d\n", self->write_pos, self->item_size);
    memcpy(data, self->item[self->write_pos++], self->item_size);
    self->fill--;
    self->write_pos = self->write_pos % self->item_count;
    RK_AUDIO_LOG_D("self->write_pos:%d after \n", self->write_pos);
    if (self->state & AUDIO_QUEUE_STATE_WAIT_WRITABLE)
    {
        self->state &= (~AUDIO_QUEUE_STATE_WAIT_WRITABLE);
        audio_semaphore_give(self->write_sem);
    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

int audio_queue_receive_back(struct audio_player_queue *self, void *data)
{
    return RK_AUDIO_SUCCESS;
}

int audio_queue_stop(struct audio_player_queue *self)
{
    audio_mutex_lock(self->lock);
    if (!(self->state | AUDIO_QUEUE_STATE_STOPPED))
    {
        if (self->state & AUDIO_QUEUE_STATE_WAIT_READABLE)
        {
            self->state &= (~AUDIO_QUEUE_STATE_WAIT_READABLE);
            audio_semaphore_give(self->read_sem);
        }
        if (self->state & AUDIO_QUEUE_STATE_WAIT_WRITABLE)
        {
            self->state &= (~AUDIO_QUEUE_STATE_WAIT_WRITABLE);
            audio_semaphore_give(self->write_sem);
        }
        self->state |= AUDIO_QUEUE_STATE_STOPPED;
    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

bool audio_queue_is_full(struct audio_player_queue *self)
{
    bool is_full = false;
    audio_mutex_lock(self->lock);
    if (self->fill >= self->item_count)
    {
        is_full = true;
    }
    audio_mutex_unlock(self->lock);
    return is_full;
}

bool audio_queue_is_empty(struct audio_player_queue *self)
{
    bool is_empty = false;
    audio_mutex_lock(self->lock);
    if (self->fill == 0)
    {
        is_empty = true;
    }
    audio_mutex_unlock(self->lock);
    return is_empty;
}

int audio_queue_finish(struct audio_player_queue *self)
{
    audio_mutex_lock(self->lock);
    if (self->state & AUDIO_QUEUE_STATE_WAIT_READABLE)
    {
        self->state &= (~AUDIO_QUEUE_STATE_WAIT_READABLE);
        self->state |= AUDIO_QUEUE_STATE_FINISHED;
        audio_semaphore_give(self->read_sem);
    }
    else
    {
        self->state |= AUDIO_QUEUE_STATE_FINISHED;
    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

int audio_queue_peek(struct audio_player_queue *self, void *data)
{
    audio_mutex_lock(self->lock);
    if (self->fill)
    {
        memcpy(data, self->item[self->write_pos], self->item_size);
    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

void audio_queue_destroy(struct audio_player_queue *self)
{
    if (self)
    {
        audio_mutex_destroy(self->lock);
        audio_semaphore_destroy(self->read_sem);
        audio_semaphore_destroy(self->write_sem);
        while (self->item_count--)
        {
            audio_free(self->item[self->item_count]);
        }
        audio_free(self);
    }
}

struct audio_player_stream   *audio_stream_create(size_t size)
{
    struct audio_player_stream *stream = (struct audio_player_stream *) audio_calloc(1, sizeof(struct audio_stream));
    if (stream)
    {
        stream->buf_size = size;
        stream->buf = (char *)audio_calloc(1, size);
        stream->lock = audio_mutex_create();
        stream->read_sem = audio_semaphore_create();
        stream->write_sem = audio_semaphore_create();
    }
    return stream;
}

int audio_stream_start(struct audio_player_stream *self)
{
    audio_mutex_lock(self->lock);
    self->fill = 0;
    self->read_pos = 0;
    self->write_pos = 0;
    self->state = AUDIO_STREAM_STATE_RUNNING;
    audio_mutex_unlock(self->lock);

    return RK_AUDIO_SUCCESS;
}

int audio_stream_read(struct audio_player_stream *self, char *data, size_t data_len)
{
    size_t buf_size;
    size_t fill;
    size_t read_pos = 0;
    size_t read_size = 0;
    size_t remaining_size = data_len;
    size_t total_read_size = 0;
    char *read_buf = data;
    if (data_len)
    {
        while (1)
        {
            audio_mutex_lock(self->lock);
            if (self->state & AUDIO_STREAM_STATE_STOPPED)
            {
                total_read_size = -1;
                audio_mutex_unlock(self->lock);
                break;
            }

            buf_size = self->buf_size;
            fill = self->fill;
            read_pos = self->read_pos;
            read_size = buf_size - read_pos;
            if (read_size > buf_size)
            {
                read_size = buf_size;
            }
            if (read_size >= remaining_size)
            {
                read_size = remaining_size;
            }
            if (read_size <= fill)
            {
                memcpy(read_buf, self->buf + read_pos, read_size);
                self->fill -= read_size;
                self->read_pos = (self->read_pos + read_size) % buf_size;
                if (self->state & AUDIO_STREAM_STATE_WAIT_WRITABLE)
                {
                    self->state &= (~AUDIO_STREAM_STATE_WAIT_WRITABLE);
                    audio_semaphore_give(self->write_sem);
                }
                remaining_size -= read_size;
                total_read_size += read_size;
                read_buf += read_size;
                audio_mutex_unlock(self->lock);
                if (!remaining_size)
                {
                    break;
                }
            }
            else if (self->state & AUDIO_STREAM_STATE_FINISHED)
            {
                if (!fill)
                {
                    self->state &= (~AUDIO_STREAM_STATE_FINISHED);
                    audio_mutex_unlock(self->lock);
                    break;
                }
                read_size = fill;
                memcpy(read_buf, self->buf + read_pos, read_size);
                self->fill -= read_size;
                self->read_pos = (self->read_pos + read_size) % buf_size;
                remaining_size -= read_size;
                total_read_size += read_size;
                read_buf += read_size;
                audio_mutex_unlock(self->lock);//add by cherry
                break;
            }
            else
            {
                self->state |= AUDIO_STREAM_STATE_WAIT_READABLE;
                audio_mutex_unlock(self->lock);
                audio_semaphore_take(self->read_sem);
            }
        }
    }
    return total_read_size;
}

int audio_stream_read2(struct audio_player_stream *self, char *data, size_t data_len)
{
    return RK_AUDIO_SUCCESS;
}

int audio_stream_write(struct audio_player_stream *self, const char *data, size_t data_len)
{
    size_t buf_size;
    size_t fill;
    size_t write_pos;
    size_t write_size;
    size_t remaining_size = data_len;
    size_t total_write_size = 0;
    const char *write_buf = data;
    if (data_len)
    {
        while (1)
        {
            audio_mutex_lock(self->lock);
            if (!(self->state & AUDIO_STREAM_STATE_RUNNING))
            {
                audio_mutex_unlock(self->lock);
                return 0;
            }
            if (self->state & AUDIO_STREAM_STATE_STOPPED)
            {
                total_write_size = -1;
                audio_mutex_unlock(self->lock);
                break;
            }

            buf_size = self->buf_size;
            fill = self->fill;
            write_pos = self->write_pos;
            write_size = buf_size - write_pos;
            if (write_size >= buf_size)
            {
                write_size = buf_size;
            }
            if (write_size >= remaining_size)
            {
                write_size = remaining_size;
            }
            if (write_size <= buf_size - fill)
            {
                memcpy(self->buf + write_pos, write_buf, write_size);
                self->fill += write_size;
                self->write_pos = (self->write_pos + write_size) % buf_size;
                if (self->state & AUDIO_STREAM_STATE_WAIT_READABLE)
                {
                    self->state &= (~AUDIO_STREAM_STATE_WAIT_READABLE);
                    audio_semaphore_give(self->read_sem);
                }
                remaining_size -= write_size;
                total_write_size += write_size;
                write_buf += write_size;
                audio_mutex_unlock(self->lock);
                if (!remaining_size)
                {
                    break;
                }
            }
            else
            {
                self->state |= AUDIO_STREAM_STATE_WAIT_WRITABLE;
                audio_mutex_unlock(self->lock);
                audio_semaphore_take(self->write_sem);
            }
        }
    }
    return total_write_size;

}

int audio_stream_write2(struct audio_player_stream *self, const char *data, size_t data_len)
{
    return RK_AUDIO_SUCCESS;
}

int audio_stream_finish(struct audio_player_stream *self)
{
    audio_mutex_lock(self->lock);
    if (self->state & AUDIO_STREAM_STATE_WAIT_READABLE)
    {
        self->state &= (~AUDIO_STREAM_STATE_WAIT_READABLE);
        self->state |= (AUDIO_STREAM_STATE_FINISHED);
        audio_semaphore_give(self->read_sem);
    }
    else
    {
        self->state |= (AUDIO_STREAM_STATE_FINISHED);
    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

int audio_stream_stop(struct audio_player_stream *self)
{
    audio_mutex_lock(self->lock);
    if (!(self->state & AUDIO_STREAM_STATE_STOPPED))
    {
        if (self->state & AUDIO_STREAM_STATE_WAIT_WRITABLE)
        {
            self->state &= (~AUDIO_STREAM_STATE_WAIT_WRITABLE);
            self->state |= AUDIO_STREAM_STATE_STOPPED;
            audio_semaphore_give(self->write_sem);
        }
        else
        {
            self->state |= AUDIO_STREAM_STATE_STOPPED;
        }
        if (self->state & AUDIO_STREAM_STATE_WAIT_READABLE)
        {
            self->state &= (~AUDIO_STREAM_STATE_WAIT_READABLE);
            audio_semaphore_give(self->read_sem);
        }

    }
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

int audio_stream_stop2(struct audio_player_stream *self)
{
    return RK_AUDIO_SUCCESS;
}

int audio_stream_reset(struct audio_player_stream *self)
{
    audio_mutex_lock(self->lock);
    self->fill = 0;
    self->read_pos = 0;
    self->write_pos = 0;
    self->state = AUDIO_STREAM_STATE_IDLE;
    audio_mutex_unlock(self->lock);
    return RK_AUDIO_SUCCESS;
}

void audio_stream_destroy(struct audio_player_stream *self)
{
    if (self)
    {
        audio_mutex_destroy(self->lock);
        audio_semaphore_destroy(self->read_sem);
        audio_semaphore_destroy(self->write_sem);
        audio_free(self->buf);
        audio_free(self);
    }
}

int check_native_audio_type(char *target, char **type)
{
    int audio_file = 0;

    char *p = strrchr(target, '.');
    if (!p)
    {
        RK_AUDIO_LOG_E("target is NULL");
        return RK_AUDIO_FAILURE;
    }
    if (0 == strcasecmp(p, ".pcm"))
    {
        *type = "pcm";
    }
    else if (0 == strcasecmp(p, ".wav"))
    {
        *type = "wav";
    }
    else if (0 == strcasecmp(p, ".mp3"))
    {
        *type = "mp3";
    }
    else if (0 == strcasecmp(p, ".aac"))
    {
        *type = "aac";
    }
    else if (0 == strcasecmp(p, ".flac"))
    {
        *type = "flac";
    }
    else if (0 == strcasecmp(p, ".amr"))
    {
        *type = "amr";
    }
    else
    {
        RK_AUDIO_LOG_E("type is NULL\n");
        return RK_AUDIO_FAILURE;
    }

    return RK_AUDIO_SUCCESS;
}
