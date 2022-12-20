#include <utils.h>
#include <stdio.h>
#include <unistd.h>

void print_progress_bar(double progress) {
	int i = 0;

	if(progress < 0) {
		return;
	}

	if(progress > 100) {
		progress = 100;
	}

	printf("\r[");

	for(i = 0; i < 50; i++) {
		if(i < progress / 2) {
			printf("=");
		} else {
			printf(" ");
		}
	}

	printf("] %3.1f%%", progress);

	fflush(stdout);

	if(progress == 100) {
		printf("\n");
	}
}

int progress_cb(irecv_client_t client, const irecv_event_t* event) {
	if (event->type == IRECV_PROGRESS) {
		print_progress_bar(event->progress);
	}

	return 0;
}

irecv_client_t get_client() {
    int i = 0;
    int64_t ecid = 0;

    irecv_client_t client = NULL;
	for (i = 0; i <= 5; i++) {
		irecv_error_t err = irecv_open_with_ecid(&client, ecid);
		if (err == IRECV_E_UNSUPPORTED) {
			fprintf(stderr, "ERROR: %s\n", irecv_strerror(err));
			return NULL;
		}
		else if (err != IRECV_E_SUCCESS)
			sleep(1);
		else
			break;

		if (i == 5) {
			fprintf(stderr, "ERROR: %s\n", irecv_strerror(err));
			return NULL;
		}
	}

    return client;
}

int send_file(const char *filename) {
    irecv_client_t client = get_client();
    irecv_event_subscribe(client, IRECV_PROGRESS, &progress_cb, NULL);
	irecv_error_t error = irecv_send_file(client, filename, 1);
    irecv_close(client);
    if (error != 0) {
	    printf("%s\n", irecv_strerror(error));
        return error;
    }
    return 0;
}

int run_command(const char *command) {
    irecv_client_t client = get_client();
    irecv_error_t error = irecv_send_command(client, command);
    irecv_close(client);
    if (error != 0) {
	    printf("%s\n", irecv_strerror(error));
        return error;
    }
    return 0;
}

int set_env(const char *key, const char *value) {
	char setenv_cmd[1024] = "";
	sprintf(setenv_cmd, "setenv %s %s", key, value);
	if (run_command(setenv_cmd) != 0) {
		return 1;
	}
	if (run_command("saveenv") != 0) {
        return 1;
    }
    return 0;
}