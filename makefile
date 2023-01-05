proje: File_client File_manager

file_client: File_client.c
	gcc File_client.c -o file_client
	
writef: File_manager.c
	gcc File_manager.c -o file_manager
			
clear: 
	rm -rf *o clear
