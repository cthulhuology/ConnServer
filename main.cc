// main.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
#include <sys/wait.h>
#include "server.h"
#include "env.h"

bool kill_all = false;
bool reload_all = false;
bool restart_all = false;

void kill_handle(int s) {
	cerr << "Killing children" << endl;
	kill_all = true;
}

void reload_handle(int s) {
	cerr << "Reloading children" << endl;
	reload_all = true;
}

void restart_handle(int s) {
	cerr << "Restarting children" << endl;
	restart_all = true;
}


int
main ( int argc, char** argv )
{
	int i;
	if (! Env::env["CSPID"].empty()) {
		FILE* fp = fopen(Env::env["CSPID"].c_str(),"w");
		if (fp) {
			fprintf(fp,"%d",getpid());
		}
		fclose(fp);
	}
	if (! Env::env["CSLOG"].empty()) {
		int fd = open(Env::env["CSLOG"].c_str(),O_CREAT|O_TRUNC|O_WRONLY,0644);
		if (fd == -1) {
			cerr << "Failed to redirect STDERR to file " << argv[i+1] << endl;
		}
		dup2(fd,2);
	}
	signal(SIGPIPE,SIG_IGN);

	for (;;) {
		cerr << "Spawining slave" << endl;
		int cpid = fork();
		if (cpid < 0) {
			cerr << "Failed to spawn server" << endl;
			exit(1);
		}
		if (cpid == 0) {
			cerr << "Respawn server" << endl;
			Server::init(argc,argv);
			Server::serve();
			Server::finalize();
		} else {
			signal(SIGHUP,reload_handle);
			signal(SIGINT,restart_handle);
			signal(SIGQUIT,kill_handle);
			signal(SIGTERM,kill_handle);
			int status;
			for (;;) {
				// Wait on child and refork if dies 
				if (0 > waitpid(cpid,&status,0)) {
					cerr << "Child died unexpectedly!" << endl;
					kill(cpid,SIGQUIT);
					break;
				}
				// TERM,QUIT signal exit cleanly
				if (kill_all) {
					cerr << "Killing child" << endl;
					kill(cpid,SIGQUIT);
					exit(0);
				}
				// INT signal kill child and refork
				if (restart_all) {
					restart_all = false;
					cerr << "Restarting child" << endl;
					kill(cpid,SIGQUIT);
					break;
				}
				// HUP signal HUP child to for module reload
				if (reload_all) {
					reload_all = false;
					cerr << "Reloading child" << endl;
					kill(cpid,SIGHUP);
					continue;
				}
			}
		}
	}
}

