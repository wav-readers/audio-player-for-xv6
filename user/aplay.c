#include <assert.h>

#include "user/user.h"
#include "user/wav.h"

//#define DEBUG

// Parsed command representation
#define ERROR -1
#define HELP 0
#define OPEN 1
#define VOLUME 2
#define PAUSE 3
#define PLAY 4
#define STOP 5
#define QUIT 99

#define MAXARGS 3
#define MAX_CM_LEN 100
#define MAX_CM_NAME_LEN 10  // MAX_COMMAND_NAME_LENGTH

struct cmd {
  int type;             // 指令类型
  char *argv[MAXARGS];  // 参数。第0个参数是指令名
  int argn;
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
 * @param s 形如"open [file]"的完整指令。以\n结尾
 */
void parsecmd(char *s, struct cmd *cmd) {
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ERROR;

  char *c = s;
  int argn;
  for (argn = 0;; c++) {
    switch (*c) {
      case '\n':
        *c = '\0';
        goto determineType;
      case ' ':
        *c = '\0';
        break;
      default:
        if (c == s || *(c - 1) == '\0') {
          cmd->argv[argn] = c;
          argn++;
          break;
        }
        break;
    }
  }
determineType : {
  cmd->argn = argn;

#ifdef DEBUG
  printf("argn: %d\n", cmd->argn);
  for (int i = 0; i < cmd->argn; i++) {
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
}

void runcmd(struct cmd *cmd) {
  if (cmd->type == ERROR) {
    fprintf(2, "输入的指令有误！\n");
    return;
  }

  switch (cmd->type) {
    case HELP:
      printf(
          "open: "
          "打开音频文件并播放。"
          "播放中时也可使用，程序将停止播放当前音频，转而播放新的音频。\n");
      printf("  open file\n");
      printf("  file为音频文件地址。\n");

      printf("volume: 设置音量。\n");
      printf("  volume n\n");
      printf("  n为目标音量。\n");

      printf("pause: 暂停播放。\n");
      printf("  pause\n");
      printf("play: 继续播放。\n");
      printf("  play\n");
      printf("stop: 停止播放。可用open指令播放下一个音频。\n");
      printf("  stop\n");

      printf("quit: 退出播放器。\n");
      printf("  quit\n");
      break;
    // TODO
    case OPEN:
      break;
    case VOLUME:
      break;
    case PAUSE:
      break;
    case PLAY:
      break;
    case STOP:
      break;

    case QUIT:
      exit(0);
    default:
      // assert(0);
      break;
  }
}

int main(int argc, char *argv[]) {
  printf("Audio Player 1.0\n");
  printf("Type \"help\" for more information.\n");

  char buf[MAX_CM_LEN];
  struct cmd *cmd = (struct cmd *)malloc(sizeof(struct cmd));
  while (getcmd(buf, sizeof(buf)) >= 0) {
    parsecmd(buf, cmd);
    runcmd(cmd);
  }

  exit(0);
}