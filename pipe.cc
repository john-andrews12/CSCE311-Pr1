# include <stdio.h>
# include <unistd.h>
# include <main.h>
# include <stdlib.h>

//file input
void
read_pipe (int file)
{
	FILE *stream;
	int c;
	stream = fdopen (file, "r");
	while ((c = fgetc (stream)) != EOF)
	{
	putchar(c);
	}
	fclose (stream);
}

//this is where text is written to the pipe

//file output
void
write_pipe (int file)
{
	FILE *stream;
	stream = fdopen (file, "w");
	fprintf (stream, " "); //OUTPUT HERE
	fclose(stream);
}

int main(void)
{
	pid_t pid;
	int pipefd[2];

	//create the pipe
	if (pipe (pipefd))
		{
			fprintf(stderr, "Pipe Failed.\n");
			return EXIT_FAILURE;
		}

	//create child process
	pid = fork();
	if (pid ==(pid_t) 0)
	{
		close (pipefd[1]);
		read_pipe (pipefd[0]);
		return EXIT_SUCCESS;
	}

	else if (pid < (pid_t) 0)
	{
		//fork Failed
		fprintf (stderr, "Fork failed. \n");
		return EXIT_FAILURE;
	}

	else
	{
		close (pipefd[0]);
		write_pipe (pipefd[1]);
		return EXIT_SUCCESS;
	}
}
