// audio player interface
// 音频播放器入口。控制器，即用户界面。

#include "user/aplaycore.h"
#include "user/user.h"

#define NDEBUG

enum Command { ERROR, HELP, OPEN, VOLUME, SPEED, PAUSE, RESUME, STOP, QUIT };

#define MAXARGS 3
#define MAX_CM_LEN 100

struct ApAudioPlayInfo *apinfo;

void showHelp() {
  printf("open:\n");
  printf("    open <file path>       open the file and play.\n");
  printf("                           File already open will be closed first.\n");

  printf("volume: default max volume is 100.\n");
  printf("    volume                 print current volume\n");
  printf("    volume <target volume> set volume\n");
  printf("speed: default speed is 1.0\n");
  printf("    speed <target speed>   set play speed\n");

  printf("pause:\n");
  printf("    pause                  pause playing\n");
  printf("resume:\n");
  printf("    resume                 resume playing\n");
  printf("stop:\n");
  printf("    stop                   stop playing\n");

  printf("quit:\n");
  printf("    quit                   quit the Audio Player\n");
}

struct cmd {
  enum Command type;    // 指令类型
  char *argv[MAXARGS];  // 参数。第0个参数是指令名
  int argc;
};

/**
 * @brief 读取用户输入的指令
 * @param buf 存输入的指令的缓冲区
 * @param nbuf 缓冲区大小
 * @return int 成功0，失败-1
 */
int getcmd(char *buf, int nbuf) {
  printf(">>> ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if (buf[0] == 0) return -1;
  return 0;
}

/**
 * @brief 翻译指令字符串s为cmd结构体
 * @param s 形如"open <file>"的完整指令。以\n结尾
 */
void parsecmd(char *s, struct cmd *cmd) {
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ERROR;

  char *c = s;
  int argc;
  for (argc = 0;; c++) {
    if (*c == '\n') {
      *c = '\0';
      break;
    }
    if (*c == ' ') {
      *c = '\0';
      continue;
    }
    if (c == s || *(c - 1) == '\0') {
      cmd->argv[argc] = c;
      argc++;
    }
  }
  cmd->argc = argc;

#ifndef NDEBUG
  printf("argc: %d\n", cmd->argc);
  for (int i = 0; i < cmd->argc; i++) {
    printf("argv[%d]: %s\n", i, cmd->argv[i]);
  }
#endif

  char *cm_name = cmd->argv[0];
  if (strcmp(cm_name, "help") == 0) {
    cmd->type = HELP;
  } else if (strcmp(cm_name, "open") == 0) {
    cmd->type = OPEN;
  } else if (strcmp(cm_name, "volume") == 0) {
    cmd->type = VOLUME;
  } else if (strcmp(cm_name, "speed") == 0) {
    cmd->type = SPEED;
  } else if (strcmp(cm_name, "pause") == 0) {
    cmd->type = PAUSE;
  } else if (strcmp(cm_name, "resume") == 0) {
    cmd->type = RESUME;
  } else if (strcmp(cm_name, "stop") == 0) {
    cmd->type = STOP;
  } else if (strcmp(cm_name, "quit") == 0) {
    cmd->type = QUIT;
  } else {
    cmd->type = ERROR;
  }
}

void runcmd(struct cmd *cmd) {
  switch (cmd->type) {
    case ERROR:
      fprintf(2, "invalid command\n");
      return;

    case HELP:
      showHelp();
      return;

    case OPEN:
      if (cmd->argc != 2) {
        fprintf(2, "%s <file path>\n", cmd->argv[0]);
        return;
      }
      if (apinfo->hasOpened) apCloseAudio(apinfo);
      if (apOpenAudio(cmd->argv[1], apinfo) >= 0) {
        apShowAudioInfo(apinfo);
        apReadDecode(apinfo);
        apSetPlay(1, apinfo);
      }
      return;

    case VOLUME:
      if (cmd->argc > 2) {
        fprintf(2, "%s <target volume>\n", cmd->argv[0]);
        return;
      }

      if (cmd->argc == 1) {
        printf("current volume: %d\n", apinfo->volume);
        return;
      }

      if (apSetVolume(atoi(cmd->argv[1]), apinfo) >= 0) {
        printf("current volume: %d\n", apinfo->volume);
      }
      return;

    case SPEED:
      if (cmd->argc != 2) {
        fprintf(2, "%s <target speed>\n", cmd->argv[0]);
        return;
      }
      if (!apinfo->hasOpened) {
        fprintf(2, "no file open\n");
        return;
      }
      if (apSetSpeed(atof(cmd->argv[1]), apinfo) >= 0) {
        printf("current speed: %s\n", cmd->argv[1]);
      }
      return;

    case PAUSE:
      apinfo->hasOpened ? apSetPlay(0, apinfo) : fprintf(2, "no file open\n");
      return;
    case RESUME:
      apinfo->hasOpened ? apSetPlay(1, apinfo) : fprintf(2, "no file open\n");
      return;
    case STOP:
      apinfo->hasOpened ? apCloseAudio(apinfo) : fprintf(2, "no file open\n");
      return;
    case QUIT:
      if (apinfo->hasOpened) apCloseAudio(apinfo);
      exit(0);
  }
}

int main(int argc, char *argv[]) {
  printf("Audio Player 1.0\n");
  printf("Type \"help\" for more information.\n");

  apinfo = ApAudioPlayInfo();
  char buf[MAX_CM_LEN];
  struct cmd *cmd = (struct cmd *)malloc(sizeof(struct cmd));
  while (getcmd(buf, sizeof(buf)) >= 0) {
    parsecmd(buf, cmd);
    runcmd(cmd);
  }

  exit(0);
}
