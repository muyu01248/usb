    libusb_device **devs;  
     libusb_device_handle *dev_handle;  
     libusb_context *ctx=NULL;  
     struct  libusb_device_descriptor desc;  
     int r;  
     ssize_t cnt;  
     r=libusb_init(&ctx);  
     if(r<0)  
     {  
         //printf("init error");  
         QMessageBox::about(this,"haha","initialerror");  
         return ;  
     }  
     libusb_set_debug(ctx,3);  
     //cnt=libusb_get_device_list(ctx,&devs);  
     dev_handle=libusb_open_device_with_vid_pid(ctx,VID,PID);  
     if(dev_handle==NULL)  
         QMessageBox::about(this,"haha","open my dev error");  
     else  
         QMessageBox::about(this,"haha","open my dev success");  
     //libusb_free_device_list(devs,1);  
     if(libusb_kernel_driver_active(dev_handle,0)==1)  
     {  
         QMessageBox::about(this,"haha","kernel driver active");  
         if(libusb_detach_kernel_driver(dev_handle,0)==0)  
             QMessageBox::about(this,"haha","kernel driver detached");  
     }  
     r=libusb_claim_interface(dev_handle,0);  
     if(r<0)  
     {  
         QMessageBox::about(this,"haha","cannot claim interface");  
         return ;  
     }  
     QMessageBox::about(this,"haha","claimed interface");  
     // start use this usb  
     unsigned char dataOut[0x30]={0};  
     dataOut[0]=0x00;  
     dataOut[1]=0x02;  
     dataOut[2]=0x08;  
     dataOut[3]=0x00;  
     dataOut[4]=0x02;  
     dataOut[5]=0x00;  
     dataOut[6]=0xb2;  
     dataOut[7]=0x30;  
     dataOut[8]=dataOut[2]^dataOut[3]^dataOut[4]^dataOut[5]^dataOut[6]^dataOut[7];  
     //r=libusb_interrupt_transfer(dev_handle,LIBUSB_ENDPOINT_OUT, dataOut,9, &actual_length, 0);  
     //r=libusb_bulk_transfer(dev_handle,LIBUSB_ENDPOINT_OUT,dataOut,9,&actual_length,5000);  
     //此设备必须用控制传输  
     r=libusb_control_transfer(dev_handle, 0x21, 0x09, 0x0300, 0x00, dataOut+1, 0x20, 1000);  
     unsigned char dataRead[0x30]={0};  
     r=libusb_control_transfer(dev_handle, 0xa1, 0x01, 0x0300, 0x00, dataRead, 0x20, 1000);  
     if(r<0)  
     {  
         QMessageBox::about(this,"haha","transfer data error");  
     }else  
     {  
         QMessageBox::about(this,"haha","transfer data success");  
     }  
     //release everything  
     r=libusb_release_interface(dev_handle,0);  
     libusb_attach_kernel_driver(dev_handle,0);  
     libusb_close(dev_handle);  
     libusb_exit(ctx);  