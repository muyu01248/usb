// Adapted from code found at
//    http://www.linuxquestions.org/questions/linux-software-2/want-to-use-libusb-in-c-but-confused-886575/
// To be able to compile on Ubuntu, you may need to first do
//    apt-get install libusb-1.0-0-dev
// to install the libusb-1.0 library (with header file).
// Then, compile with
//    gcc -o listUSBDevices listUSBDevices.c -lusb-1.0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

int main( int argc, char *argv ) {

   libusb_device_handle * deviceHandle = NULL;                                                                                                                                         //*** libusb_device_handle

   int returnValue = libusb_init( NULL );                                                                                                                                                           //*** libusb_init()

   libusb_device ** arrayOfPointersToDevices = NULL; // an array of pointers to devices                                                             //*** libusb_device **
   ssize_t numUSBDevices = libusb_get_device_list( NULL, & arrayOfPointersToDevices );                                                             //*** libusb_get_device_list

   printf( "%lu USB devices found\n", numUSBDevices );

   ssize_t deviceIndex = 0;
   while ( deviceIndex < numUSBDevices ) {
      printf( "[device %lu]\n", deviceIndex );

      libusb_device * devicePointer = NULL; // a pointer to a device                                                                                                          //*** libusb_device *
      devicePointer = arrayOfPointersToDevices[ deviceIndex ]; 

      struct libusb_device_descriptor deviceDescriptor;                                                                                                                                   //** libusb_device_descriptor
      returnValue = libusb_get_device_descriptor( devicePointer, & deviceDescriptor );                                                                       //** libusb_get_device_descriptor
      if ( returnValue != LIBUSB_SUCCESS )
         break;

      printf( "  idVendor: %u\n", deviceDescriptor.idVendor );                                                                                                                        //** idvendor
      printf( "  idProduct: %u\n", deviceDescriptor.idProduct );                                                                                                                     //** idProduct

      deviceHandle = NULL;
      returnValue = libusb_open( devicePointer, & deviceHandle );                                                                                                                //** libusb_open
      if ( returnValue != LIBUSB_SUCCESS ) {
         printf("    We don't have permission to open the device. Maybe try running this program as root.\n");
         deviceHandle = NULL;
      }

      // Get the string associated with iManufacturer index.
      const int STRING_LENGTH = 256;                                                                                                                                                                                                            //** 
      unsigned char stringDescription[ STRING_LENGTH ];
      printf( "  iManufacturer = %d\n", deviceDescriptor.iManufacturer );
      if ( deviceHandle != NULL && deviceDescriptor.iManufacturer > 0 ) {
         returnValue = libusb_get_string_descriptor_ascii( deviceHandle, deviceDescriptor.iManufacturer, stringDescription, STRING_LENGTH );    //** libusb_get_string_descriptor_ascii
         if ( returnValue < 0 )
            break;
         printf( "    string = %s\n",  stringDescription );
      }

      // Get string associated with iProduct index.
      printf( "  iProduct = %d\n", deviceDescriptor.iProduct );
      if ( deviceHandle != NULL && deviceDescriptor.iProduct > 0 ) {
         returnValue = libusb_get_string_descriptor_ascii( deviceHandle, deviceDescriptor.iProduct, stringDescription, STRING_LENGTH );
         if ( returnValue < 0 )
            break;
         printf( "    string = %s\n", stringDescription );
      }

      // Get string associated with iSerialNumber index.
      printf( "  iSerialNumber = %d\n", deviceDescriptor.iSerialNumber );
      if ( deviceHandle != NULL && deviceDescriptor.iSerialNumber > 0 ) {
         returnValue = libusb_get_string_descriptor_ascii( deviceHandle, deviceDescriptor.iSerialNumber, stringDescription, STRING_LENGTH );
         if ( returnValue < 0 )
            break;
         printf( "    string = %s\n", stringDescription );
      }

      // Close and try next one.
      if ( deviceHandle != NULL ) {
         libusb_close( deviceHandle );                                                                                                                                                       //** libusb_close
         deviceHandle = NULL;
      }
      deviceIndex ++;
   }

   if ( deviceHandle != NULL ) {
      // Close device if left open due to break out of loop on error.
      libusb_close( deviceHandle );
   }

   libusb_exit( NULL );

   printf( "Done\n" );
   return 0;
}