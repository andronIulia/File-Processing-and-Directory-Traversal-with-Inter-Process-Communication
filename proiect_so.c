#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/wait.h>

#define BUFSIZE 4096
#define PATH_SIZE 4097

char buffer[BUFSIZE];
char buf[BUFSIZE],buf2[BUFSIZE];
char fis[BUFSIZE];

void conversie_bmp(char *cale) { //conversie imagine bmp
    int bmp_file;
    if ((bmp_file= open(cale, O_RDWR)) < 0) {
        printf("Eroare la deschiderea fișierului BMP\n");
        exit(44);
    }
    
    int width, height;
    if(lseek(bmp_file, 18, SEEK_SET)<0){
    	printf("Eroare la mutarea cursorului\n");
    	exit(7);
    }
    /*if(read(bmp_file, &width, sizeof(int))<=0){
    	printf("Eroare la citirea fisierului bmp\n");
    	exit(7);
    }
    if(read(bmp_file, &height, sizeof(int))<=0){
    	printf("Eroare la citirea fisierului bmp\n");
    	exit(7);
    }*/
    read(bmp_file, &width, sizeof(int));
    read(bmp_file, &height, sizeof(int));
    int data_offset;
    if(lseek(bmp_file, 10, SEEK_SET)<0){
    	printf("Eroare la mutarea cursorului\n");
    	exit(7);
    }
    /*if(read(bmp_file, &data_offset, sizeof(int))<=0){
    	printf("Eroare la citirea fisierului bmp\n");
    	exit(7);
    }*/
    read(bmp_file, &data_offset, sizeof(int));
    if(lseek(bmp_file, data_offset, SEEK_SET)<0){
    	printf("Eroare la mutarea cursorului\n");
    	exit(7);
    }

    unsigned char pixel[3];

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            /*if(read(bmp_file, pixel, sizeof(unsigned char) * 3)<=0){
            	printf("Eroare la citirea fisierului bmp\n");
    		exit(7);
            }*/
            read(bmp_file, pixel, sizeof(unsigned char) * 3);
            unsigned char conv_gri = 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];

            if(lseek(bmp_file, -3, SEEK_CUR)<0){
            	printf("Eroare la mutarea cursorului\n");
    		exit(7);
            }
            if(write(bmp_file, &conv_gri, sizeof(unsigned char))<0){
            	printf("Eroare la scriere in fisier\n");
    		exit(7);
            }
            if(write(bmp_file, &conv_gri, sizeof(unsigned char))<0){
            	printf("Eroare la scriere in fisier\n");
    		exit(7);
            }
            if(write(bmp_file, &conv_gri, sizeof(unsigned char))<0){
            	printf("Eroare la scriere in fisier\n");
    		exit(7);
            }

            if(lseek(bmp_file, 3 - sizeof(unsigned char) * 3, SEEK_CUR)<0){
            	printf("Eroare la mutarea cursorului\n");
    		exit(7);
            }
        }

        int padding = (4 - (width * 3) % 4) % 4;
        if(lseek(bmp_file, padding, SEEK_CUR)<0){
        	printf("Eroare la mutarea cursorului\n");
    		exit(7);
        }
    }


    if(close(bmp_file)<0){
    	printf("Eroare la inchiderea fisierului\n");
    	exit(45);
    }
}

void afisare_drepturi(char f,struct stat info){ //functie pentru calcularea si scrierea drepturilor in fisier
	int s=sprintf(buffer,"Drepturi de acces user:%c%c%c.\n",(info.st_mode & S_IRUSR) ? 'R' : '-',(info.st_mode & S_IWUSR) ? 'W' : '-',(info.st_mode & S_IXUSR) ? 'X' : '-');
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(29);
	}
	s=sprintf(buffer,"Drepturi de acces grup:%c%c%c.\n",(info.st_mode & S_IRGRP) ? 'R' : '-',(info.st_mode & S_IWGRP) ? 'W' : '-',(info.st_mode & S_IXGRP) ? 'X' : '-');
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(30);
	}
	s=sprintf(buffer,"Drepturi de acces alti utilizatori:%c%c%c.\n",(info.st_mode & S_IROTH) ? 'R' : '-',(info.st_mode & S_IWOTH) ? 'W' : '-',(info.st_mode & S_IXOTH) ? 'X' : '-');	
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(31);
	}
}

void afisare_dir(char f, char cale[],struct stat info){ //functie pentru scrierea informatiilor despre director
	int s=sprintf(buffer,"Nume director:%s.\n",cale);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(22);
	}
	s=sprintf(buffer,"Identificatorul utilizatorului:%d.\n",info.st_uid);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(23);
	}
	afisare_drepturi(f,info);
}

void afisare_leg_simb(char f,char cale[],struct stat info,char cale_link[]){ //functie pentru scrierea informatiilor despre legatura simbolica
	int s=sprintf(buffer,"Nume legatura:%s.\n",cale);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(24);
	}
	s=sprintf(buffer,"Dimensiune legatura:%ld.\n",info.st_size);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(25);
	}
	struct stat info2;
	if(lstat(cale, &info2)<0){
		printf("Eroare la lstat\n");
		exit(26);
	}
	s=sprintf(buffer,"Dimensiune fisier:%ld.\n",info2.st_size);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(27);
	}
	afisare_drepturi(f,info);
}

void afisare_fisier(char f, char cale[], struct stat info){ // functie pentru scrierea informatiilor despre fisier normal
	int s=sprintf(buffer,"Nume fisier:%s.\n",cale);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(28);
	}
	s=sprintf(buffer,"Dimensiune fisier:%ld.\n",info.st_size);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(29);
	}
	s=sprintf(buffer,"Identificatorul utilizatorului:%d.\n",info.st_uid);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(30);
	}

	time_t t_ult_modif=info.st_mtime;
	char timp[15];
	strftime(timp,15,"%d.%m.%Y",localtime(&t_ult_modif));
	s=sprintf(buffer,"Timpul ultimei modificari: %s.\n",timp);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(31);
	}
	
	s=sprintf(buffer,"Contorul de legaturi:%ld.\n",info.st_nlink);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(32);
	}
	afisare_drepturi(f,info);
}

void afisare_fisier_bmp(char f, char cale[], struct stat info){ //functie pentru scrierea informatiilor despre fisier bmp
	int s=sprintf(buffer,"Nume fisier:%s.\n",cale);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(33);
	}
	s=sprintf(buffer,"Dimensiune fisier:%ld.\n",info.st_size);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(34);
	}
	
	int bmp_file;
	if((bmp_file=open(cale,O_RDONLY))<0){
		printf("Eroare deschidere fisier bmp\n");
		exit(35);
	}
	if(lseek(bmp_file, 18, SEEK_SET)<0){
		printf("Eroare la mutarea cursorului\n");
		exit(7);
	}
    	int width, height;
    	/*if(read(bmp_file, &width, sizeof(int))<=0){
    		printf("Eroare la citirea din fisier\n");
    		exit(36); 
    	}
    	if(read(bmp_file, &height, sizeof(int))<=0){
    		printf("Eroare la citirea din fisier\n");
    		exit(37);
    	}*/
    	read(bmp_file, &width, sizeof(int));
    	read(bmp_file, &height, sizeof(int));
    	
    	if(close(bmp_file)<0){
    		printf("Eroare la inchiderea fisierului\n");
    		exit(38);
    	}
    	
	s=sprintf(buffer,"Inaltime: %d.\n",height);
	if(write(f,buffer,s)<0){
		printf("Eroare la scrierea in fisier\n");
		exit(39);
	}
	
	s=sprintf(buffer,"Lungime: %d.\n",width);
	if(write(f,buffer,s)<0){
		printf("Eroare la scrierea in fisier\n");
		exit(40);
	}
	s=sprintf(buffer,"Identificatorul utilizatorului:%d.\n",info.st_uid);
	if(write(f,buffer,s)<0){
		printf("Eroare la scriere in fisier\n");
		exit(41);
	}

	
	time_t t_ult_modif=info.st_mtime;
	char timp[15];
	strftime(timp,15,"%d.%m.%Y",localtime(&t_ult_modif));
	s=sprintf(buffer,"Timpul ultimei modificari: %s.\n",timp);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(42);
	}
	s=sprintf(buffer,"Contorul de legaturi:%ld.\n",info.st_nlink);
	if(write(f,buffer,s)<0){
		printf("Eroare de scriere in fisier\n");
		exit(43);
	}
	afisare_drepturi(f,info);
}


void parse(char *nume_dir,char *nume_dir2, int nivel,char *caract){ //functie de parcurgere director
	char f,f_p;
	DIR *dir,*dir2;
	struct dirent *in;
	char *nume;
	struct stat info;
	char cale[PATH_MAX], cale_link[PATH_MAX + 1], spatii[PATH_MAX];
	int n;
	int line_count;
    	int bytesRead;
    	memset(spatii, ' ', 2*nivel);
	spatii[2*nivel]='\0';
	if(!(dir=opendir(nume_dir))){
		printf("Eroare deschidere director\n");
		exit(2);
	}
	if(!(dir2=opendir(nume_dir2))){
		printf("Eroare deschidere director\n");
		exit(3);
	}
	if ((f= open("statistica.txt", O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR)) < 0) {
    		printf("Eroare deschidere fisier\n");
    		exit(4);
	}
	int pfd[2];
	int pfd2[2];
	pid_t pid1,pid2,wpid2,wpid1;	
	int status1,status2;
	int s;
	while((in=readdir(dir))>0){ //parcurgere director
		nume=in->d_name;
		if(strcmp(nume,".")==0 || strcmp(nume,"..")==0)
			continue;
		snprintf(cale, sizeof(cale), "%s/%s", nume_dir, nume);
		if(lstat(cale, &info)<0)
		{
			printf("Eroare la lstat\n");
			exit(5);
		}
		
		if(pipe(pfd)<0){
			printf("Eroare la crearea pipe-ului\n");
			exit(1);
		}
		if(pipe(pfd2)<0){
			printf("Eroare la crearea pipe-ului\n");
			exit(1);
		}
		//primul proces fiu
		if((pid1=fork())<0){
			printf("Eroare la fork\n");
			exit(6);
		}
		if(pid1==0){
			line_count=0;
			if(close(pfd[0])<0){
				printf("Eroare inchidere pipe\n");
				exit(5);
			}
			if(close(pfd2[0])<0){
				printf("Eroare inchidere pipe\n");
				exit(5);
			}				
			if(close(pfd2[1])<0){
				printf("Eroare inchidere pipe\n");
				exit(5);
			}
			char nume_fisier[PATH_MAX];
			snprintf(nume_fisier, sizeof(nume_fisier), "%s_statistica.txt", nume);
			char cale_out[PATH_SIZE];
			snprintf(cale_out, sizeof(cale_out), "%s/%s", nume_dir2, nume_fisier);
			if((f_p=open(cale_out,O_RDWR | O_CREAT | O_TRUNC,S_IWUSR| S_IRUSR))<0){
				printf("Eroare la deschidere fisier\n");
				exit(7);
			}
			if(S_ISDIR(info.st_mode)){
		       		afisare_dir(f_p,cale,info);
				parse(cale,nume_dir2,nivel + 1,caract);	
			}	
			else
			if(S_ISLNK(info.st_mode)){
				n = readlink(cale, cale_link, sizeof(cale_link));
				if(n<0){
					printf("Eroare la citirea legaturii\n");
					exit(8);
				}
				cale_link[n]='\0';
				afisare_leg_simb(f_p,cale,info,cale_link);
			}
			else
			if(S_ISREG(info.st_mode)){
	          		if(strstr(nume,".bmp")!=NULL)
		    			afisare_fisier_bmp(f_p,cale,info);
				else
		    			afisare_fisier(f_p,cale,info);
			}
			if (lseek(f_p, 0, SEEK_SET) < 0) {
    				printf("Eroare la mutarea cursorului\n");
    				exit(9);
			}
    			while ((bytesRead = read(f_p, buf, BUFSIZE)) > 0) {
        			for (int i = 0; i < bytesRead; i++) {
            				if (buf[i] == '\n') {
               					line_count++;
             				}
        			}
        			if(strstr(nume,".bmp")==NULL){
        				if(write(pfd[1],buf,bytesRead) < 0){
       						printf("Eroare la scriere in pipe\n");
          					exit(4);
        				}
        			}
    			}
    			if (bytesRead < 0) {
    				printf("Eroare la citire\n");
    				exit(10);
			}
			if(close(f_p)<0){
				printf("Eroare la inchiderea fisierului\n");
				exit(11);
			}	
            		if(close(pfd[1])<0){
            			printf("Eroare inchidere pipe\n");
           			exit(5);
          		}
            		exit(line_count);
		}
		//al doilea proces fiu
		if((pid2=fork())<0){
			printf("Eroare la fork\n");
			exit(6);
		}
		if(pid2==0){
			if(strstr(nume,".bmp")!=NULL){
				if(close(pfd2[0])<0){
					printf("Eroare inchidere pipe\n");
					exit(5);
				}
				if(close(pfd[0])<0){
					printf("Eroare inchidere pipe\n");
					exit(5);
				}
				if(close(pfd[1])<0){
            				printf("Eroare inchidere pipe\n");
            				exit(5);
            			}
				
				if(close(pfd2[1])<0){
					printf("Eroare inchidere pipe\n");
					exit(5);
				}
				conversie_bmp(cale);
				exit(6);
			}
			else if(strstr(nume,".bmp")==NULL){
			 	if(close(pfd[1])<0){
            				printf("Eroare inchidere pipe\n");
          				exit(5);
            			}
			 	if(close(pfd2[0])<0){
					printf("Eroare inchidere pipe\n");
					exit(5);
				}
				if(dup2(pfd[0],0)<0){
					printf("Eroare la redirectarea intrarii\n");
					exit(5);
				}
				if(dup2(pfd2[1],1)<0){
					printf("Eroare la redirectarea iesirii\n");
					exit(5);
				}
				if(close(pfd[0])<0){
					printf("Eroare inchidere pipe\n");
					exit(5);
				}
				if(close(pfd2[1])<0){
					printf("Eroare inchidere pipe\n");
					exit(5);
				}
				execlp("bash","bash","script.sh",caract,NULL);
				printf("eroare la exec\n");
				exit(2);
			}
					
		}
		//procesul parinte
		if(close(pfd[0])<0){
			printf("Eroare inchidere pipe\n");
			exit(5);
		}
		if(close(pfd[1])<0){
            		printf("Eroare inchidere pipe\n");
          		exit(5);
            	}
		
		if(close(pfd2[1])<0){
			printf("Eroare de inchidere pipe\n");
			exit(5);
		}
		int numar;
		if(strstr(nume,".bmp")==NULL){
		if(read(pfd2[0],buf2,sizeof(buf2))<0){
			printf("Eroare la citirea din pipe\n");
			exit(5);
		}	
		if(sscanf(buf2, "%d", &numar)<0){
			printf("Eroare la sscanf\n");
			exit(9);
		}
		}
		if(close(pfd2[0])<0){
			printf("Eroare de inchidere pipe\n");
			exit(5);
		}
		wpid1 = waitpid(pid1,&status1,0);
		wpid2 = waitpid(pid2,&status2,0);
		if(WIFEXITED(status1)){
			s=snprintf(fis, sizeof(fis), "%d linii scrise in fisierul de statistica\n",WEXITSTATUS(status1));
			if (write(f,fis, s) < 0) {
    				printf("Eroare de scriere in fisier\n");
    				exit(13);
			}				
			s=snprintf(fis, sizeof(fis), "S-a incheiat procesul cu PID: %d si Exit Code: %d\n", wpid1, WEXITSTATUS(status1));
			if (write(f,fis, s) < 0) {
    				printf("Eroare de scriere in fisier\n");
    				exit(13);
			}
		}
		else{
			s=snprintf(fis, sizeof(fis), "S-a incheiat procesul abnormal cu PID: %d \n", wpid1);
			if (write(f,fis, s) < 0) {
 				printf("Eroare de scriere in fisier\n");
    				exit(13);
			}
		}
		if(strstr(nume,".bmp")==NULL){
			s=snprintf(fis, sizeof(fis), "Au fost identificate in total %d propozitii corecte care contin caracterul %s\n",numar,caract);
			if (write(f,fis, s) < 0) {
    				printf("Eroare de scriere in fisier\n");
    				exit(13);
			}
		}
		if(WIFEXITED(status2)){
			s=snprintf(fis, sizeof(fis), "S-a incheiat procesul cu PID: %d si Exit Code: %d\n\n", wpid2, WEXITSTATUS(status2));
			if (write(f,fis, s) < 0) {
    				printf("Eroare de scriere in fisier\n");
    				exit(13);
			}
		}
		else{
			s=snprintf(fis, sizeof(fis), "S-a incheiat procesul abnormal cu PID: %d \n", wpid2);
			if (write(f,fis, s) < 0) {
    				printf("Eroare de scriere in fisier\n");
    				exit(14);
			}
		}
			
			
	}
	if(close(f)<0){
		printf("Eroare la inchiderea fisierului\n");
		exit(18);
	}
	if(closedir(dir)<0){
		printf("Eroare la inchiderea directorului\n");
		exit(19);
	}
	if(closedir(dir2)<0){
		printf("Eroare la inchiderea directorului\n");
		exit(20);
	}
}

int main(int argc, char *argv[])
{
	if(argc!=4){
		printf("Numar incorect de argumente\n");
		exit(1);
	}
	parse(argv[1],argv[2],0,argv[3]);
	return 0;

}
