
#include "log_writer.h"

void log_writer_init(R(LogWriter*) logWriter, R(const char*) ipcPath)
{
    basic_init(B(logWriter), EQP_SOURCE_ID_LOG_WRITER, NULL);
    
    shm_viewer_init(&logWriter->shmViewer);
    shm_viewer_open(B(logWriter), &logWriter->shmViewer, ipcPath, sizeof(IpcBuffer));
    
    log_thread_init(B(logWriter), &logWriter->thread);
    log_thread_start(B(logWriter), &logWriter->thread);
}

void log_writer_deinit(R(LogWriter*) logWriter)
{
    thread_send_stop_signal(B(logWriter), T(&logWriter->thread));
    thread_wait_until_stopped(T(&logWriter->thread));
    log_thread_deinit(&logWriter->thread);
    
    shm_viewer_close(&logWriter->shmViewer);
    
    basic_deinit(B(logWriter));
}

void log_writer_main_loop(R(LogWriter*) logWriter)
{
    R(IpcBuffer*) ipc = shm_viewer_memory_type(&logWriter->shmViewer, IpcBuffer);
    
    for (;;)
    {
        // Block until we receive an IPC packet
        ipc_buffer_wait(B(logWriter), ipc);
        
        for (;;)
        {
            IpcPacket packet;
            
            if (!ipc_buffer_read(B(logWriter), ipc, &packet))
                break;
            
            if (ipc_packet_opcode(&packet) == ServerOp_Shutdown)
                return;
            
            // Forward to the logging thread
            log_thread_post_message(B(logWriter), &logWriter->thread, &packet);
        }
    }
}
