/*shell made by Theodoros Chalkidis*/
/*csd4198 email: csd4198@csd.uoc.gr*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#define BUFFER 1024
#define HALFBUFFER 512

/*DEBUG PRINT COMMENT THE DEFINE TO STOP IT*/
#define DEBUG
#ifdef DEBUG
#define DPRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINTF(...) ;
#endif

#define STREQ(x, s) (strcmp(x, s) == 0)

char *trimwhitespace(char *str);

/*for coloring the printf*/

void change_printf_color()
{
  printf("\033[1;33m");
}
void change_printf_color_red()
{
  printf("\033[0;31m");
}
void change_printf_color_purple()
{
  printf("\033[1;36m");
}
void reset()
{
  printf("\033[0m");
}

/*returns 0 if no redirection found 1 if found > 2 if found >> */
/*returns -1 for < and -2 << and -3 if >< or <> found*/
int has_redirection(char *command_line)
{
  
  char *checking = command_line;
  int total = 0;
  int flag = 0;
  while (*checking != '\0' && checking != NULL)
  {
    if (*checking == '>')
    {
      if (flag != -1)
      {
        flag = 1;
      }
      else
      {
        printf("redirection given wrong , first < and then > \n");
        return -3;
      }

      total++;
    }
    else if (*checking == '<')
    {
      if (flag != 1)
      {
        flag = -1;
      }
      else
      {
        printf("redirection given wrong , first > and then < \n");
        return -3;
      }

      total--;
    }
    checking++;
  }
  return total;
}

int count_of_commands(char **command_table)
{
  int count_of_commands = 0;
  while (command_table[count_of_commands] != NULL)
  {
    count_of_commands++;
  }
  return count_of_commands;
}

/*purpose is to split any string into multiple strings whenever i see the spl_char*/
/*if character is blank space then whatever is in "" returns as a whole argument*/
/*returns null if something went wrong also adds a guard NULL guard node*/
char **split_to_commands(char *unprosseced_Buffer, char spl_char)
{
  int red = 0;
  char **commands_buffer = malloc(512 * sizeof(char *));
  char *copy_of_buffer = malloc(BUFFER * sizeof(char));
  char *start_of_token;
  char *current_check;
  int command_line = 0;
  /*i make a copy of the original buffer so i dont mess initial files*/
  strcpy(copy_of_buffer, unprosseced_Buffer);
  copy_of_buffer = trimwhitespace(copy_of_buffer);
  start_of_token = copy_of_buffer;
  current_check = copy_of_buffer;

  /*only when we check for white space there is a point to search for redirection*/
  if (spl_char == ' ')
  {
    red = has_redirection(unprosseced_Buffer);
  }
  

  while (1)
  {
    /*if i reach the end i copy the last token to the command buffer*/
    if (*current_check == '\0')
    {
      commands_buffer[command_line] = malloc(1024 * sizeof(char));
      strcpy(commands_buffer[command_line], start_of_token);
      /*trimming the white spaces back and forth*/
      commands_buffer[command_line] = trimwhitespace(commands_buffer[command_line]);
      command_line++;
      break;
    }

    /*if i found an argument that is inside "" and not a redirection file*/
    /* at the time i break commands from*/
    /*arguments i replace them and take it as a whole*/
    /*also red is 0 if a redirection is found coz first i get rid of the symbols and tthe proceed*/
    /*to clean the "" for the file and the split the commands*/
    else if (*current_check == '"' && spl_char == ' ' && red == 0)
    {
      *current_check = '\0';
      start_of_token = ++current_check;
      /*i search for the second " or \0 */
      while (*current_check != '"' && *current_check != '\0')
      {
        current_check++;
      }

      /*checking to see if it was double quotation as needed*/
      if (*current_check == '\0')
      {
        change_printf_color_red();
        printf("ERROR quotation opend and left open \n");
        reset();
        return NULL;
      }
      /*i mark the end of the quotation*/
      *current_check = '\0';
      commands_buffer[command_line] = malloc(1024 * sizeof(char));

      strcpy(commands_buffer[command_line], start_of_token);
      /*trimming the white spaces back and forth*/
      commands_buffer[command_line] = trimwhitespace(commands_buffer[command_line]);

      command_line++;

      start_of_token = current_check + 1;
    }

    /*if i find the split char i make it \0 so the strcpy starts at the*/
    /*start of the token and ends there */
    else if (*current_check == spl_char)
    {
      *current_check = '\0';

      commands_buffer[command_line] = malloc(1024 * sizeof(char));

      strcpy(commands_buffer[command_line], start_of_token);
      /*trimming the white spaces back and forth*/
      commands_buffer[command_line] = trimwhitespace(commands_buffer[command_line]);
      command_line++;
      current_check++;
      /*then i search the next non space character*/
      while (*current_check == ' ')
      {
        current_check++;
      }
      /*and start the new token*/
      start_of_token = current_check;
    }
    else
    {
      current_check++;
    }
  }
  /*adds a guard null node so the count of commands can work*/
  if (*commands_buffer[command_line - 1] == '\0')
  {

    commands_buffer[command_line - 1] = NULL;
  }
  else
  {
    commands_buffer[command_line] = malloc(1024 * sizeof(char));
    commands_buffer[command_line] = NULL;
  }

  return commands_buffer;
}

/*purpose is to remove blank spaces after each command*/
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while (isspace((unsigned char)*str))
    str++;

  if (*str == 0) // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

/*erases the >> > << < from a command line, doesnt check for >< <<< >>> wrong usage*/
char *erase_redirection(char *str)
{
  char *tmp = str;
  while (*tmp != '\0' && tmp != NULL)
  {
    if (*tmp == '>' || *tmp == '<')
    {
      *tmp = '\0';
    }
    tmp++;
  }
  return str;
}

/*takes the command and alters it removing the path of the redirection and returning it alone*/
char *find_path_red(char *str)
{
  char *start = str;
  char *end = str;
  int flag = 0;
  /*ignores every quotation before redirection symbol, those are took care elsewhere*/
  while (*end != '\0')
  {
    if (*end == '>' || *end == '<')
    {
      end++;
      start = end;
      break;
    }

    end++;
  }

  while (*end != '\0' && end != NULL)
  {
    if (*end == '"' && flag == 0)
    {
      start = end + 1;
      *end = '\0';
      flag++;
    }
    else if (*end == '"')
    {
      *end = '\0';
    }

    end++;
  }
  char *return_str;
  return_str = malloc(strlen(start) * sizeof(char));
  strcpy(return_str, start);

  return return_str;
}

void execute_redirection(char *line_of_command)
{
  /*copying the buffer so i can alter it*/
  char *line_of_command_copy = malloc(BUFFER * sizeof(char));
  strcpy(line_of_command_copy, line_of_command);
  /*type of redirection*/
  int type_of_redirection = has_redirection(line_of_command_copy);

  char *path = find_path_red(line_of_command_copy);
  line_of_command_copy = erase_redirection(line_of_command_copy);
  line_of_command_copy = trimwhitespace(line_of_command_copy);

  /*i break the file path/name as an argument*/
  char **command_arguments = split_to_commands(line_of_command_copy, ' ');

  /*i start the fork process*/
  pid_t pid;
  int status;
  pid = fork();
  int fd;

  /*if fork was success the pid of the child is 0*/
  /*so i want to execv with the broken arguments i made*/
  if (pid == 0)
  {
    switch (type_of_redirection)
    {
    case -1:

      fd = open(path, O_RDONLY);

      if (dup2(fd, 0) == -1)
      {
        perror("something went wrong with this :");
        return;
      }
      close(fd);

      break;
    case 1:

      fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);

      if (dup2(fd, 1) == -1)
      {
        perror("something went wrong with this :");
        return;
      }
      close(fd);

      break;
    case 2:
      fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
      if (dup2(fd, 1) == -1)
      {
        perror("something went wrong with this :");
        return;
      }
      close(fd);

      break;

    default:
      printf("something went terribly wrong \n");
      return;
    }
    /*in the command_arguments[0] i store the command and in the rest the arguments*/
    execvp(command_arguments[0], command_arguments);
    exit(0);
  } /*in case fork failed for some reason*/
  else if (pid == -1)
  {
    printf("something went wrong with fork\n");
    return;
  }
  else
  {
    close(fd);
    /*inside the parrent fork i have to wait for the child to finish*/
    /*meaning wait for the child whose process ID is equal to the value of pid.*/
    if (waitpid(pid, &status, 0) > 0)
    {
      /*wifexited macro returns true if child ended with success*/
      /*wexitstatus returns the exit status of the child*/
      if (WIFEXITED(status) && !WEXITSTATUS(status))
      {
        
        return;
      }

      else if (WIFEXITED(status) && WEXITSTATUS(status)) /*in case something didnt work as expected*/
      {
        if (WEXITSTATUS(status) == 127)
        {
          // execv failed
          printf("execv failed\n");
        }
        else
          //it did terminate but didnt return 0 at exit , aka something unexpected happend
          printf("program terminated normally, but returned a non-zero status\n");
      }
      else
        //wifexited didnt return true so the child did not terminate correctly
        printf("program didn't terminate normally\n");
    }
    else
    {
      // waitpid() failed
      printf("waitpid() failed\n");
    }
  }
  
}

/*purpose is to take the line of command and split it among command and arguments*/
void execute_line_of_command(char *line_of_command)
{
  /*i split the arguments from the command itself*/

  char **command_arguments = split_to_commands(line_of_command, ' ');

  /*if the command is the cd*/
  if (STREQ(command_arguments[0], "cd"))
  {
    /*if no arguments given*/
    if (command_arguments[1] == NULL)
    {
      if (chdir(".") != 0)
      {
        printf("something went wrong with cd command:\033[0;31m %s\033[0m\n", line_of_command);
      }
    }
    else
    {
      if (chdir(command_arguments[1]) != 0)
      {
        printf("something went wrong with cd command:\033[0;31m %s\033[0m\n", line_of_command);
      }
    }
    return;
  }

  /*checking of redirection if found calls execute redirection and returns*/
  /*otherwise continues the programm*/
  int redireriction_tmp = has_redirection(line_of_command);
  if (redireriction_tmp < 3 && redireriction_tmp > -2 && redireriction_tmp != 0)
  {

    execute_redirection(line_of_command);

    return;
  }
  else if (redireriction_tmp != 0)
  {
    printf("something went wrong with the redirection characters \n");
    return;
  }
  /*i start the fork process*/
  pid_t pid;

  int status;
  pid = fork();

  /*if fork was success the pid of the child is 0*/
  /*so i want to execv with the broken arguments i made*/
  if (pid == 0)
  {
    /*in the command_arguments[0] i store the command and in the rest the arguments*/
    execvp(command_arguments[0], command_arguments);
    exit(0);
  } /*in case fork failed for some reason*/
  else if (pid == -1)
  {
    printf("something went wrong with fork\n");
    return;
  }
  else
  {
    /*inside the parrent fork i have to wait for the child to finish*/
    /*meaning wait for the child whose process ID is equal to the value of pid.*/
    if (waitpid(pid, &status, 0) > 0)
    {
      /*wifexited macro returns true if child ended with success*/
      /*wexitstatus returns the exit status of the child*/
      if (WIFEXITED(status) && !WEXITSTATUS(status))
        return;

      else if (WIFEXITED(status) && WEXITSTATUS(status)) /*in case something didnt work as expected*/
      {
        if (WEXITSTATUS(status) == 127)
        {
          // execv failed
          printf("execv failed\n");
        }
        else
          //it did terminate but didnt return 0 at exit , aka something unexpected happend
          printf("program terminated normally, but returned a non-zero status\n");
      }
      else
        //wifexited didnt return true so the child did not terminate correctly
        printf("program didn't terminate normally\n");
    }
    else
    {
      // waitpid() failed
      printf("waitpid() failed\n");
    }
  }
}

/*execute pipes , line_of_commands comes broken to pipes*/
void execute_pipes(char **line_of_commands)
{
  /*counts the total commands , pipes are total - 1 */
  int total_commands_of_pipes = count_of_commands(line_of_commands);

  /*creates the pipe output with (total-1)*2 in/outs for example*/
  /*for n=2 we need 1 in 1 out for n=3 we need 2ins and 2outs*/
  /* ignoring for the first command the in and for the last the out*/
  int pipes[(count_of_commands(line_of_commands) - 1)][2];

  /*creates a for loop for every command of the pipe that with the right use of ifs*/
  /*will determine if we are in the first last or middle pipes*/
  for (int i = 0; i < total_commands_of_pipes; i++)
  {
    /*checking for redirection*/
    int flag_for_redirection = 0;
    int type_of_redirection = has_redirection(line_of_commands[i]);
    /*the only redirection allowed is input at first pipe output at last pipe*/
    if (i == 0 && type_of_redirection == -1)
    {
      flag_for_redirection = 1;
    }
    else if (i == total_commands_of_pipes - 1 && type_of_redirection > 0)
    {
      flag_for_redirection = 1;
    }
    else if (type_of_redirection != 0)
    {
      change_printf_color_red();
      printf("redirection in the middle of the pipe not allowed\n");
      printf("unxpected errors will occur\n");
      reset();
      return;
    }
    /*if redirection found*/
    char *path = malloc(BUFFER * sizeof(char));
    int red_fd;
    if (flag_for_redirection)
    {
      path = find_path_red(line_of_commands[i]);
      line_of_commands[i] = erase_redirection(line_of_commands[i]);
      line_of_commands[i] = trimwhitespace(line_of_commands[i]);
    }

    /*creating pipes at place i*/
    pipe(pipes[i]);
    /*i start the fork process*/
    pid_t pid;

    int status;
    pid = fork();

    /*if fork was success the pid of the child is 0*/
    /*so i want to execv with the broken arguments i made*/
    if (pid == 0)
    {
      /*in the first loop we are at the first pipe*/
      if (i == 0)
      {
        char **command_arguments = split_to_commands(line_of_commands[i], ' ');
        /*if redirection found*/
        if (flag_for_redirection)
        {
          red_fd = open(path, O_RDONLY);

          if (dup2(red_fd, 0) == -1)
          {
            perror("something went wrong with this :");
            return;
          }
          close(red_fd);
        }

        dup2(pipes[i][1], 1);
        /*closing every pipe coz i am in fork process*/
        close(pipes[i][0]);
        close(pipes[i][1]);
        execvp(command_arguments[0], command_arguments);
        exit(0);
      }
      else if (i < total_commands_of_pipes - 1) /*middle loops*/
      {
        char **command_arguments = split_to_commands(line_of_commands[i], ' ');
        dup2(pipes[i - 1][0], 0);
        dup2(pipes[i][1], 1);
        /*closing only the write pipe and read pipe*/
        close(pipes[i - 1][0]);
        close(pipes[i][1]);
        execvp(command_arguments[0], command_arguments);
        exit(0);
      }
      else /*in the last loop*/
      {
        /*closing everything except the last pipe where i read from*/
        char **command_arguments = split_to_commands(line_of_commands[i], ' ');

        /*if redirection found*/
        if (flag_for_redirection)
        {
          /*checking the type of the redirection*/
          if (type_of_redirection == 1)
          {
            red_fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0644);
          }
          else
          {
            red_fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
          }

          if (dup2(red_fd, 1) == -1)
          {
            perror("something went wrong with this :");
            return;
          }
          close(red_fd);
        }
        dup2(pipes[total_commands_of_pipes - 2][0], 0);
        /*closing every pipe*/
        for (int k = 0; k < total_commands_of_pipes - 1; k++)
        {
          close(pipes[k][0]);
          close(pipes[k][1]);
        }
        execvp(command_arguments[0], command_arguments);
        exit(0);
      }
    }
    else if (pid == -1)
    {
      printf("something went wrong with forking\n");
      return;
    }
    else
    {
      /*parent needs to close write pipe*/
      close(pipes[i][1]);
      wait(&status);
      if (i != 0)
      {
        close(pipes[i - 1][0]);
      }
    }
  }
}

void print_start(void)
{
  change_printf_color_red();
  printf("SHELL\n");
  printf("Made by theodoros chalkidis csd4198\n");
  reset();
  printf("This shell mimics as best as possible the wsl\n");
  printf("for example the commands cd .. | whoami on wsl dont change directory\n");
  printf("bur rather print the whoami result only , this shell does the same thing\n");
  change_printf_color();
  printf("for the redirection to work use %cfile.txt %c\n", '"', '"');
  reset();
  change_printf_color_red();
  printf("KNOWN BUGS :\n");
  reset();
  printf("this shell doesnt support double redirection , for example sort <sample.txt >result.txt \n");
  printf("to make this work you can call sort < sample.txt | sort > result.txt\n");
  printf("type help for this panel to re-appear\n");
  printf("\n\nalso when giving arguments with file name such us test 2.txt works if the name with the space\n");
  printf("is given inside quotes like %ctest 2.txt%c \n", '"', '"');
  printf("also arguments must preceed the path inside the quotes \n");
}

int main(int argc, char **argv)
{
  char *buffer;
  char directory_buffer[BUFFER];
  execute_line_of_command("clear");
  print_start();
  change_printf_color();
  printf("%s@cs345sh:\033[0;34m%s$  ", getlogin(), getcwd(directory_buffer, BUFFER));
  reset();
  buffer = readline("");

  while (1)
  {
    char **commands_buffer = split_to_commands(buffer, ';');
    char **line_of_command;
    int max_iterations_i = count_of_commands(commands_buffer);
    /*i break the commands first by ;*/
    for (size_t i = 0; i < max_iterations_i; i++)
    {

      /*i break the commands into pipes */
      line_of_command = split_to_commands(commands_buffer[i], '|');

      /*if a pipe is detected i proceed the execution customly*/

      if (line_of_command[1] != NULL)
      {
        execute_pipes(line_of_command);
      }
      else
      {
        int max_iterations_j = count_of_commands(line_of_command);
        for (size_t j = 0; j < max_iterations_j; j++)
        {
          /*if given exit as an command or cd i need to treat it specialy*/
          if (STREQ(line_of_command[j], "exit"))
          {
            change_printf_color_purple();
            printf("have a nice life \n");
            reset();
            exit(0);
          }
          else if (STREQ(line_of_command[j], "help"))
          {
            print_start();
          }
          else
          {

            execute_line_of_command(line_of_command[j]);
            
          }
        }
      }
    }
    change_printf_color();
    printf("\n%s@cs345sh:\033[0;34m%s$  ", getlogin(), getcwd(directory_buffer, BUFFER));
    reset();
    free(buffer);
    buffer = readline("");
  }

  return 0;
}