/**
 * @file srftp.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 10 June 2015
 * 
 * @brief Simplified File Transfer Protocol server logic implementation
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * The purpose of the server in SimpleFTP is to wait for clients to connect
 * and to be able to exchange metadata with them and receive files (in binary
 * mode). All communication with client shall be done in separate thread.
 * 
 * Usage: srftp server_port max_file_size
 * where:
 *		server_port: the port of SimpleFTP Server application to listen to.
 *		max_file_size: maximum file size this server can accept.
 */
 

 int main(int argc, char const *argv[])
 {
 	struct args;

 	validateArgs(argc, argv, &args);

 	
 	
 	return 0;
 }