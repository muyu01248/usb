    #include <errno.h>  
    #include <pthread.h>  
    #include <semaphore.h>  
    #include <signal.h>  
    #include <string.h>  
    #include <stdio.h>  
    #include <stdlib.h>  
    #include <windows.h>  
    #include "libusb.h"  
      
    #define EP_IN           (6 | LIBUSB_ENDPOINT_IN)  
    #define EP_OUT      (2 | LIBUSB_ENDPOINT_OUT)  
      
      
    typedef enum {  
        SAVE_IMAGE      = 0x55,  
        UPLOAD_IMAGE    = 0xAA  
    }BULK_REQCODE;  
      
    typedef enum {  
        SAVE_IMAGE_VAL  = 0x00,  
        UPLOAD_B_VAL    = 0x00,  
        UPLOAD_G_VAL    = 0x01,  
        UPLOAD_R_VAL    = 0x02  
    }BULK_VALUE;  
      
    static struct libusb_device_handle *devh = NULL;  
    static unsigned char imgbuf_save[8];  
    static unsigned char imgbuf_upload[8];  
    static unsigned char imgbuf_R[2048*240];  
    static unsigned char imgbuf_G[2048*240];  
    static unsigned char imgbuf_B[2048*240];  
    static struct libusb_transfer *img_save = NULL;  
    static struct libusb_transfer *img_R = NULL;  
    static struct libusb_transfer *img_G = NULL;  
    static struct libusb_transfer *img_B = NULL;  
    static struct libusb_transfer *img_R_upload = NULL;  
    static struct libusb_transfer *img_G_upload = NULL;  
    static struct libusb_transfer *img_B_upload = NULL;  
    static int img_idx = 0;  
    static volatile sig_atomic_t do_exit = 0;  
    static int ii=0;  
      
    static pthread_t poll_thread;  
    static sem_t exit_sem;  
      
    static void request_exit(sig_atomic_t code)  
    {  
        do_exit = code;  
        sem_post(&exit_sem);  
    }  
      
    static void *poll_thread_main(void *arg)    //轮询是否main——thread结束  
    {  
        int r = 0;  
        printf("poll thread running\n");  
      
        while (!do_exit) {  
            struct timeval tv = { 1, 0 };  
            r = libusb_handle_events_timeout(NULL, &tv);    //处理任何待以处理的事件  
            if (r < 0) {  
                request_exit(2);  
                break;  
            }  
        }  
      
        printf("poll thread shutting down\n");  
        return NULL;  
    }  
      
    static int find_dpfp_device(void)  
    {  
        devh = libusb_open_device_with_vid_pid(NULL, 0x04b4, 0x8623);  
        return devh ? 0 : -EIO;  
    }  
      
    static void LIBUSB_CALL cb_img(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "upload_img_R_over callback : " << transfer->status << std::endl;  
    }  
      
    static void LIBUSB_CALL save_img(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "save Image callback : " << transfer->status << std::endl;  
    }  
      
    static void LIBUSB_CALL upload_img_B(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "upload_img_B callback : " << transfer->status << std::endl;  
    }  
      
    static void LIBUSB_CALL upload_img_B_over(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "upload_img_B_over callback : " << transfer->status << std::endl;  
    }  
      
    static void LIBUSB_CALL upload_img_G(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "upload_img_G callback : " << transfer->status << std::endl;  
    }  
      
    static void LIBUSB_CALL upload_img_G_over(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "upload_img_G_over callback : " << transfer->status << std::endl;  
    }  
      
    static void LIBUSB_CALL upload_img_R(struct libusb_transfer *transfer)  
    {  
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {  
            fprintf(stderr, "img transfer status %d?\n", transfer->status);  
            request_exit(2);  
            return;  
        }  
      
        std::cout << "upload_img_R callback : " << transfer->status << std::endl;  
      
    }  
      
    //由于该设备处理多命令异步发送过来的时候，只处理最后一条  
    static int LIBUSB_CALL submit_trans()  
    {  
        int r=0;  
      
        r = libusb_submit_transfer(img_save);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_save);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        r = libusb_submit_transfer(img_B_upload);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_B_upload);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        r = libusb_submit_transfer(img_B);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_B);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        r = libusb_submit_transfer(img_G_upload);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_G_upload);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        r = libusb_submit_transfer(img_G);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_G);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        r = libusb_submit_transfer(img_R_upload);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_R_upload);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        r = libusb_submit_transfer(img_R);   //提交一个传输并立即返回  
        if (r < 0) {  
            libusb_cancel_transfer(img_R);   //异步取消之前提交传输  
            printf("1");  
        }  
      
        return r;  
    }  
      
    static int alloc_transfers(void)  
    {  
        img_save = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_save)  
            return -ENOMEM;  
        img_R = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_R)  
            return -ENOMEM;  
        img_R_upload = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_R_upload)  
            return -ENOMEM;  
        img_G = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_G)  
            return -ENOMEM;  
        img_G_upload = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_G_upload)  
            return -ENOMEM;  
        img_B = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_B)  
            return -ENOMEM;  
        img_B_upload = libusb_alloc_transfer(0);    //0参数表明配置一个异步传输  
        if (!img_B_upload)  
            return -ENOMEM;  
      
        imgbuf_save[0] = (u_char)(SAVE_IMAGE);  
        imgbuf_save[1] = 0x00;  
        imgbuf_save[2] = 0x00;  
        imgbuf_save[3] = 0x00;  
        imgbuf_save[4] = (uchar)((0xff00 & 0x00f0)>>8);  
        imgbuf_save[5] = (uchar)(0x00ff & 0x00f0);  
        imgbuf_save[6] = 0x00;  
        imgbuf_save[7] = (u_char)(SAVE_IMAGE_VAL);  
        libusb_fill_bulk_transfer(img_save, devh, EP_OUT, imgbuf_save,  
            sizeof(imgbuf_save), save_img, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
      
        imgbuf_upload[0] = (u_char)UPLOAD_IMAGE;  
        imgbuf_upload[1] = 0x00;  
        imgbuf_upload[2] = 0x00;  
        imgbuf_upload[3] = 0x00;  
        imgbuf_upload[4] = (uchar)((0xff00 & 0x00f0)>>8);  
        imgbuf_upload[5] = (uchar)(0x00ff & 0x00f0);  
        imgbuf_upload[6] = 0x00;  
        imgbuf_upload[7] = (u_char)UPLOAD_B_VAL;  
        libusb_fill_bulk_transfer(img_B_upload, devh, EP_OUT, imgbuf_upload,  
            sizeof(imgbuf_upload), upload_img_B, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
        libusb_fill_bulk_transfer(img_B, devh, EP_IN, imgbuf_B,  
            sizeof(imgbuf_B), upload_img_B_over, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
      
        imgbuf_upload[0] = (u_char)UPLOAD_IMAGE;  
        imgbuf_upload[1] = 0x00;  
        imgbuf_upload[2] = 0x00;  
        imgbuf_upload[3] = 0x00;  
        imgbuf_upload[4] = (uchar)((0xff00 & 0x00f0)>>8);  
        imgbuf_upload[5] = (uchar)(0x00ff & 0x00f0);  
        imgbuf_upload[6] = 0x00;  
        imgbuf_upload[7] = (u_char)UPLOAD_G_VAL;  
        libusb_fill_bulk_transfer(img_G_upload, devh, EP_OUT, imgbuf_upload,  
            sizeof(imgbuf_upload), upload_img_G, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
        libusb_fill_bulk_transfer(img_G, devh, EP_IN, imgbuf_G,  
            sizeof(imgbuf_G), upload_img_G_over, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
      
        imgbuf_upload[0] = (u_char)UPLOAD_IMAGE;  
        imgbuf_upload[1] = 0x00;  
        imgbuf_upload[2] = 0x00;  
        imgbuf_upload[3] = 0x00;  
        imgbuf_upload[4] = (uchar)((0xff00 & 0x00f0)>>8);  
        imgbuf_upload[5] = (uchar)(0x00ff & 0x00f0);  
        imgbuf_upload[6] = 0x00;  
        imgbuf_upload[7] = (u_char)UPLOAD_R_VAL;  
        libusb_fill_bulk_transfer(img_R_upload, devh, EP_OUT, imgbuf_upload,  
            sizeof(imgbuf_upload), upload_img_R, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
        libusb_fill_bulk_transfer(img_R, devh, EP_IN, imgbuf_R,  
            sizeof(imgbuf_R), cb_img, NULL, 5000);   //该辅助函数来填充所需的libusb_transfer批量传输字段  
      
        return 0;  
    }  
      
    int main(void)  
    {  
        libusb_init(NULL);  //初始libusb库  
      
        int r = find_dpfp_device();  
        if(r!=0)  
        {  
            printf("%s\n", "not found dev" );  
            return 0;  
        }  
      
        r = libusb_claim_interface(devh, 0);    //声明设备接口  
        if (r < 0) {  
            fprintf(stderr, "usb_claim_interface error %d %s\n", r, strerror(-r));  
            goto out;  
        }  
        std::cout << "claimed interface" << std::endl;  
      
        r = pthread_create(&poll_thread, NULL, poll_thread_main, NULL); //子线程poll_thread_main用来处理USB事件  
        if (r)  
            goto out;  
      
        r = alloc_transfers();  //配置一个异步bulk传输和一个中断传输，先alloc然后fill_XX_transfer  
        if (r < 0) {  
            request_exit(1);  
            pthread_join(poll_thread, NULL);    //函数pthread_join用来等待一个线程的结束  
            goto out;  
        }  
      
        r = submit_trans(); //submit一个传输  
        if (r < 0) {  
            request_exit(1);  
            pthread_join(poll_thread, NULL);    //失败等待线程结束  
            goto out;  
        }  
      
        while (!do_exit)  
            sem_wait(&exit_sem);    //等待退出信号量  
      
        printf("shutting down...\n");  
        pthread_join(poll_thread, NULL);  
      
    out_release:  
        libusb_release_interface(devh, 0);  
    out:  
        libusb_close(devh);  
        libusb_exit(NULL);  
        sem_destroy(&exit_sem);  
        return r >= 0 ? r : -r;  
    }  