#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void func(char **comm,int no,int last,int fd[][2],int in,int infile,int out,int outfile,int bg)
{
	pid_t pid;
	pid=fork();
	int i,j,k,a=5,status;
	if (pid<0){ 
		printf("Child Process Not Created\n");
		exit(-1);
	}
	else if (pid==0){
		if(no==0 && no!=last-1)
			dup2(fd[no][1],1);
		if(no==last-1 && no!=0)
			dup2(fd[no-1][0],0);
		if(no>0 && no<last-1){
			dup2(fd[no-1][0],0);
			dup2(fd[no][1],1);
		}
		if(in!=0){
			dup2(infile,0);
			close(infile);
		}
		if(out!=0){
			dup2(outfile,1);
			close(outfile);
		}
		a=execvp(*comm, comm);
		if(a<0) { 
			printf("%s: command not found\n",comm[0]);
			exit(-1);
		}
	}
	else if(bg==0){
		while(wait(&status)!=pid);
	}
}
char* change(char *wd,char *user,char *pwd){
	int i,c=strlen(wd),k=strlen(user);
	char arr[64]={'/','h','o','m','e','/'},sarr[64];
	for(i=0;i<strlen(user);i++)
		arr[i+6]=user[i];
	for(i=0;i<6+strlen(user) && i<strlen(wd);i++)
		sarr[i]=wd[i];
	if(strcmp(sarr,arr)==0){
		for(i=0;i<c-6-k;i++)
			pwd[i+1]=wd[i+6+k];
		pwd[0]='~';
		pwd[i+1]='\0';
	}
	else{
		for(i=0;i<strlen(wd);i++)
			pwd[i]=wd[i];
		pwd[i]='\0';
	}
}
int main()
{
	char  arr[1024],toopen[1024],c,line[1024],*comm[64],*comm2[64],wd[1024],pwd[1024],*token,*token2,user[1024],sys[1024],prpwd[1024];
	const char del[10]={' ','\t','\n','\0'};
	int r,i,k,j,bg;
	getcwd(wd, sizeof(wd));
	getcwd(pwd, sizeof(pwd));
	getlogin_r(user,sizeof(user));
	gethostname(sys,sizeof(sys));
	change(wd,user,prpwd);
	while(1){
		printf("%s@%s:%s$ ",user,sys,prpwd);
		bg=0;
		i=0;k=0;
		while(comm2[k]!=NULL){
			comm2[k]=NULL;
			k++;
		}
		k=0;
		while((c=getchar())!='\n'){
			line[i]=c;
			i++;
		}
		line[i]='\0';
		if(line[i-1]=='&'){
			bg=1;
			line[i-1]='\0';
		}
		token2=strtok(line,"|");
		while(token2!=NULL){
			comm2[k]=token2;
			k++;
			token2=strtok(NULL,"|");
		}
		int pipes=k,fd[100][2];
		k=0;
		for(i=0;i<pipes;i++)
			pipe(fd[i]);
		for(i=0;i<pipes;i++){
			int out=0,in=0,infile,outfile;
			while(comm[k]!=NULL){
				comm[k]=NULL;
				k++;
			}
			for(j=0;j<strlen(comm2[i]);j++){
				if(comm2[i][j]=='>'){
					k=j+1;
					while(comm2[i][k]==' ')
						k++;
					j=k;
					for(;k<strlen(comm2[i]) && comm2[i][k]!='>' && comm2[i][k]!='<' && comm2[i][k]!=' ';k++)
						arr[k-j]=comm2[i][k];
					arr[k-j]='\0';
					outfile=open(arr,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					out=1;
				}
				if(comm2[i][j]=='<'){
					k=j+1;
					while(comm2[i][k]==' ')
						k++;
					j=k;
					for(;k<strlen(comm2[i]) && comm2[i][k]!='>' && comm2[i][k]!='<' && comm2[i][k]!=' '; k++)
						arr[k-j]=comm2[i][k];
					arr[k-j]='\0';
					infile=open(arr,O_RDONLY,0);
					in=1;
				}
			}
			for(j=0;j<strlen(comm2[i]);j++)
				if(comm2[i][j]=='>' || comm2[i][j]=='<')
					comm2[i][j]='\0';
			k=0;
			token=strtok(comm2[i],del);
			while(token!=NULL){
				comm[k]=token;
				k++;
				token = strtok(NULL,del);
			}
			if(comm[0]!=NULL){
				if(strcmp(comm[0],"exit")==0)
					exit(0);
				else if(strcmp(comm[0],"pwd")==0)
					printf("%s\n",wd);
				else if(strcmp(comm[0],"echo")==0)
					printf("%s\n",comm[1]);
				else if(strcmp(comm[0],"cd")==0){
					if(comm[1]!=NULL){
						if(strcmp(comm[1],"-")==0){
							chdir(pwd);
							for(r=0;r<strlen(wd);r++)
								pwd[r]=wd[r];
							pwd[r]='\0';
							getcwd(wd, sizeof(wd));
							change(wd,user,prpwd);
						}
						else if(strcmp(comm[1],"~")==0){
							chdir("/home/sanket");
							for(r=0;r<strlen(wd);r++)
								pwd[r]=wd[r];
							pwd[r]='\0';
							getcwd(wd, sizeof(wd));
							change(wd,user,prpwd);

						}
						else{
							i=chdir(comm[1]);
							if(i==0){
								for(r=0;r<strlen(wd);r++)
									pwd[r]=wd[r];
								pwd[r]='\0';
								getcwd(wd, sizeof(wd));
								change(wd,user,prpwd);
							}
							else
								printf("%s Directory does not exist\n",comm[1]);
						}
					}
					else{
						chdir("/home/sanket");
						for(r=0;r<strlen(wd);r++)
							pwd[r]=wd[r];
						pwd[r]='\0';
						getcwd(wd, sizeof(wd));
						change(wd,user,prpwd);
					}
				}
				else
					func(comm,i,pipes,fd,in,infile,out,outfile,bg);
			}
		}
	}
	return 0;
}
