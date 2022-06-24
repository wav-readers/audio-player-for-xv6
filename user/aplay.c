// audio player interface
// 音频播放器入口。控制器，即用户界面。

#include "user/aplaycore.h"
#include "user/user.h"

#define NDEBUG

enum Command { ERROR, HELP, OPEN, VOLUME, PAUSE, PLAY, STOP, QUIT };

#define MAXARGS 3
#define MAX_CM_LEN 100

struct ApAudioPlayInfo *apinfo;

void showHelp() {
  printf(
      "open: "
      "打开音频文件并播放。\n"
      "      "
      "若当前已打开音频，程序将先关闭当前音频。\n");
  printf("  usage: open <file path>\n");

  printf("volume: 设置音量。\n");
  printf("  usage: volume <target value>\n");

  printf("pause: 暂停播放。\n");
  printf("  usage: pause\n");
  printf("play: 继续播放。\n");
  printf("  usage: play\n");
  printf("stop: 停止播放。\n");
  printf("  usage: stop\n");

  printf("quit: 退出播放器。\n");
  printf("  usage: quit\n");
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
  } else if (strcmp(cm_name, "pause") == 0) {
    cmd->type = PAUSE;
  } else if (strcmp(cm_name, "play") == 0) {
    cmd->type = PLAY;
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
      break;
    case OPEN:
      if (cmd->argc != 2) {
        fprintf(2, "usage: %s <file path>\n", cmd->argv[0]);
        return;
      }
      if (apinfo->hasOpened) apCloseAudio(apinfo);
      if (apOpenAudio(cmd->argv[1], apinfo) >= 0) {
        apShowAudioInfo(apinfo);
        apReadDecode(apinfo); 
        apSetPlay(1, apinfo);
        printf("finish decoding\n");
      }
      break;
    case VOLUME:
      if (cmd->argc != 2) {
        fprintf(2, "usage: %s <target value>\n", cmd->argv[0]);
        return;
      }
      apSetVolume(atoi(cmd->argv[1]), apinfo);
      printf("current volume: %s\n", cmd->argv[1]);
      break;
    case PAUSE:
      if (apinfo->hasOpened) apSetPlay(0, apinfo);
      break;
    case PLAY:
      if (apinfo->hasOpened) apSetPlay(1, apinfo);
      break;
    case STOP:
      if (apinfo->hasOpened) apCloseAudio(apinfo);
      break;
    case QUIT:
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
