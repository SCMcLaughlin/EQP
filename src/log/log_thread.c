
#include "log_thread.h"

STRUCT_DEFINE(LogThreadFile)
{
    int     sourceId;
    FILE*   fp;
};

void log_thread_init(R(Basic*) basic, R(LogThread*) logThread)
{
    atomic_mutex_init(&logThread->inQueueMutex);
    
    logThread->inQueue      = array_create_type(basic, IpcPacket);
    logThread->writeQueue   = array_create_type(basic, IpcPacket);
    logThread->logFiles     = array_create_type(basic, LogThreadFile);
}

void log_thread_deinit(R(LogThread*) logThread)
{
    if (logThread->inQueue)
    {
        array_destroy(logThread->inQueue);
        logThread->inQueue = NULL;
    }
    
    if (logThread->writeQueue)
    {
        array_destroy(logThread->writeQueue);
        logThread->writeQueue = NULL;
    }
    
    if (logThread->logFiles)
    {
        array_destroy(logThread->logFiles);
        logThread->logFiles = NULL;
    }
}

static void log_thread_close_file(FILE* fp)
{
    char message[2048];
    time_t rawTime      = time(NULL);
    struct tm* curTime  = localtime(&rawTime);
    size_t pos          = strftime(message, sizeof(message), "[%Y-%m-%d : %H:%M:%S] ==== Log File Closed ====\n", curTime);
    
    if (pos != 0)
        fwrite(message, sizeof(byte), pos, fp);
    
    fclose(fp);
}

static void log_thread_close_all(R(LogThread*) logThread)
{
    uint32_t i;
    uint32_t n              = array_count(logThread->logFiles);
    LogThreadFile* array    = array_data_type(logThread->logFiles, LogThreadFile);
    
    for (i = 0; i < n; i++)
    {
        LogThreadFile* file = &array[i];
        
        if (file->fp)
        {
            log_thread_close_file(file->fp);
            file->fp = NULL;
        }
    }
}

static uint32_t log_thread_read_input_queue(R(LogThread*) logThread)
{
    uint32_t i;
    uint32_t n;
    
    // Check the input queue
    atomic_mutex_lock(&logThread->inQueueMutex);
    n = array_count(logThread->inQueue);
    
    if (n > 0)
    {
        IpcPacket* array = array_data_type(logThread->inQueue, IpcPacket);
        
        for (i = 0; i < n; i++)
        {
            array_push_back(B(logThread), &logThread->writeQueue, &array[i]);
        }
        
        array_clear(logThread->inQueue);
    }
    
    atomic_mutex_unlock(&logThread->inQueueMutex);
    
    return n;
}

static LogThreadFile* log_thread_get_file(R(LogThread*) logThread, int sourceId, uint32_t* index)
{
    uint32_t i;
    uint32_t n              = array_count(logThread->logFiles);
    LogThreadFile* array    = array_data_type(logThread->logFiles, LogThreadFile);
    
    for (i = 0; i < n; i++)
    {
        LogThreadFile* file = &array[i];
        
        if (file->sourceId == sourceId)
        {
            if (index)
                *index = i;
            return file;
        }
    }
    
    return NULL;
}

static void log_thread_write(R(LogThread*) logThread, R(IpcPacket*) packet)
{
    uint32_t length = ipc_packet_length(packet);
    R(byte*) data   = ipc_packet_data(packet);
    
    if (length > 0 && data != NULL)
    {
        LogThreadFile* file = log_thread_get_file(logThread, ipc_packet_source_id(packet), NULL);
        
        if (file && file->fp)
        {
            FILE* fp = file->fp;
            
            fwrite(data, sizeof(byte), length, fp);
            fputc('\n', fp);
            fflush(fp);
        }
    }
}

static FILE* log_thread_determine_filename_and_open(R(LogThread*) logThread, int sourceId)
{
    char filename[256];
    FILE* fp;
    
    switch (sourceId)
    {
    case EQP_SOURCE_ID_MASTER:
        snprintf(filename, 256, "log/master.log");
        break;
    
    case EQP_SOURCE_ID_CHAR_SELECT:
        snprintf(filename, 256, "log/char_select.log");
        break;
    
    case EQP_SOURCE_ID_LOGIN:
        snprintf(filename, 256, "log/login.log");
        break;
        
    default:
        if (sourceId >= EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET)
        {
            snprintf(filename, 256, "log/zone_cluster%i.log", sourceId - EQP_SOURCE_ID_ZONE_CLUSTER_OFFSET);
        }
        else
        {
            int zoneId  = sourceId % EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
            int instId  = sourceId / EQP_SOURCE_ID_ZONE_INSTANCE_OFFSET;
            snprintf(filename, 256, "log/zone_%s_instance%i.log", zone_short_name_by_id(zoneId), instId);
        }
        break;
    }
    
    //fixme: add logic here to check for existing large logs, to be compressed, stashed away and replaced fresh
    fp = fopen(filename, "a");
    
    if (!fp)
    {
        // Log the failure in master.log, assuming we haven't failed to open it...
        char message[2048];
        time_t rawTime      = time(NULL);
        struct tm* curTime  = localtime(&rawTime);
        size_t pos          = strftime(message, sizeof(message),
            "[%Y-%m-%d : %H:%M:%S][ERROR][log_thread_determine_filename_and_open] Could not open file for appending: ", curTime);
        
        if (pos != 0)
        {
            int wrote = snprintf(message, sizeof(message) - pos, "'%s'", filename);
            
            if (wrote > 0 && (size_t)wrote < (sizeof(message) - pos))
            {
                IpcPacket packet;
                
                pos += (size_t)wrote;
                
                ipc_packet_init(B(logThread), &packet, ServerOp_LogMessage, EQP_SOURCE_ID_MASTER, pos, message);
                array_push_back(B(logThread), &logThread->writeQueue, &packet);
            }
        }
    }
    
    return fp;
}

static void log_thread_open(R(LogThread*) logThread, R(IpcPacket*) packet)
{
    int sourceId        = ipc_packet_source_id(packet);
    LogThreadFile* old  = log_thread_get_file(logThread, sourceId, NULL); // Is it already open?
    FILE* fp;

    if (old)
        fp = old->fp;
    else
        fp = log_thread_determine_filename_and_open(logThread, sourceId);
    
    if (fp)
    {
        char message[2048];
        time_t rawTime      = time(NULL);
        struct tm* curTime  = localtime(&rawTime);
        size_t pos          = strftime(message, sizeof(message), "[%Y-%m-%d : %H:%M:%S] ==== Log File Opened ====\n", curTime);
        
        if (pos != 0)
        {
            fwrite(message, sizeof(byte), pos, fp);
            fflush(fp);
        }
        
        if (!old)
        {
            LogThreadFile file;
            
            file.sourceId   = sourceId;
            file.fp         = fp;
            
            array_push_back(B(logThread), &logThread->logFiles, &file);
        }
    }
}

static void log_thread_close(R(LogThread*) logThread, R(IpcPacket*) packet)
{
    uint32_t index;
    LogThreadFile* file = log_thread_get_file(logThread, ipc_packet_source_id(packet), &index);
    
    if (file)
    {
        if (file->fp)
        {
            log_thread_close_file(file->fp);
            file->fp = NULL;
        }
        
        array_swap_and_pop(logThread->logFiles, index);
    }
}

static void log_thread_process_write_queue(R(LogThread*) logThread)
{
    uint32_t i;
    uint32_t n          = array_count(logThread->writeQueue);
    IpcPacket* packets  = array_data_type(logThread->writeQueue, IpcPacket);
    
    for (i = 0; i < n; i++)
    {
        IpcPacket* packet = &packets[i];
        
        switch (ipc_packet_opcode(packet))
        {
        case ServerOp_LogMessage:
            log_thread_write(logThread, packet);
            break;
        
        case ServerOp_LogOpen:
            log_thread_open(logThread, packet);
            break;
        
        case ServerOp_LogClose:
            log_thread_close(logThread, packet);
            break;
        
        default:
            break;
        }
        
        ipc_packet_deinit(packet);
    }
    
    array_clear(logThread->writeQueue);
}

void log_thread_main_loop(R(Thread*) thread)
{
    R(LogThread*) logThread = (LogThread*)thread;
    
    for (;;)
    {
        thread_wait(T(logThread));
        
        for (;;)
        {
            if (!log_thread_read_input_queue(logThread))
                break;
            
            log_thread_process_write_queue(logThread);
        }
        
        if (thread_should_stop(T(logThread)))
            break;
    }
    
    log_thread_close_all(logThread);
}

void log_thread_post_message(R(Basic*) basic, R(LogThread*) logThread, R(IpcPacket*) packet)
{
    atomic_mutex_lock(&logThread->inQueueMutex);
    array_push_back(basic, &logThread->inQueue, packet);
    atomic_mutex_unlock(&logThread->inQueueMutex);
    thread_trigger(basic, T(logThread));
}
