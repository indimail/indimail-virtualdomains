/* rpclog.x: Remote msg printing protocol */
 program RPCLOG {
     version LOGVERS {
        int SEND_MESSAGE(string) = 1;
 	 } = 1;
} = 0x20000001;
