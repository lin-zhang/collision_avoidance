struct rplidar_udp_config {
	char opt_com_path[128];
	unsigned int opt_com_baudrate;
        unsigned char host_ip[15];
        unsigned int port;
	unsigned int EnableUDP;
	unsigned char debug_flag;
};
