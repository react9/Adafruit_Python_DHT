{
  "targets": [
    {
      "target_name": "rpi_dht_nodejs_addon",
      "sources": [ "rpi_dht_nodejs_addon.cc", 
				   "../source/Raspberry_Pi/pi_dht_read.c", 
				   "../source/Raspberry_Pi/pi_mmio.c", 
				   "../source/common_dht_read.c" 
				 ]
    }
  ]
}