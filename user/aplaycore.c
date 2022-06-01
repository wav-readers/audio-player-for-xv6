#include "aplaycore.h"

#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/wav.h"

struct AudioPlayInfo *AudioPlayInfo() {
  struct AudioPlayInfo *apinfo =
      (struct AudioPlayInfo *)malloc(sizeof(struct AudioPlayInfo));
  apinfo->hasOpened = 0;
  return apinfo;
}

///@todo 以下
// 和声卡交互的可考虑再单独移到新的文件去。soundcard.h, soundcard.c

int setPlay(int play, struct AudioPlayInfo *apinfo) {
  // 也许得做成系统调用
  return 0;
}

int setVolume(int volume, struct AudioPlayInfo *apinfo) {
  // 也许得做成系统调用
  return volume;
}

///@todo
int getVolume() {
  // 也许得做成系统调用
  return 0;
}

///@todo
// 也许得做成系统调用
int setSampleRate(uint sample_rate) { return 0; }

///@todo
// 也许得做成系统调用
int clearSoundCardBuffer() { return 0; }

///@todo 以上

int openAudio(const char *file, struct AudioPlayInfo *apinfo) {
  char *filext = strrchr(file, '.');
  if (strcmp(filext, ".wav") == 0) {
    apinfo->ftype = WAV;
  } else if (strcmp(filext, ".mp3") == 0) {
    fprintf(2, "将来也许会支持%s文件，但至少现在不支持……\n", filext);
    return -1;
  } else {
    fprintf(2, "invalid file extension\n");
    return -1;
  }

  int fd = open(file, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "fail to open %s\n", file);
    return -1;
  }

  switch (apinfo->ftype) {
    case WAV: {
      struct WavInfo info;
      if (readWavHead(fd, &info) < 0) {
        fprintf(2, "invalid file format");
        close(fd);
        return -1;
      }
      apinfo->wavInfo = info;
      setSampleRate(info.sample_rate);
      break;
    }
    case MP3:
      //未实现
      break;
  }

  apinfo->hasOpened = 1;
  // 填写其他信息
  apinfo->fd = fd;
  apinfo->fname = file;
  apinfo->isPlaying = 0;
  apinfo->readDecPid = -1;
  apinfo->volume = getVolume();
  return fd;
}

int beginReadDecode(struct AudioPlayInfo *apinfo) {
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fail to fork");
    return -1;
  }
  if (pid > 0) {
    apinfo->readDecPid = pid;
    return pid;
  }
  // 子进程
  void (*decode)(const char *fileData, char *decodedData) = 0;
  switch (apinfo->ftype) {
    case WAV:
      decode = decodeWav;
      break;
    case MP3:
      // 未实现
      break;
  }
  ///@todo
  // 创建缓冲区
  // question: 解码前的数据的缓冲区大小?
  // question: 解码后的数据的缓冲区大小？
  // question:
  // 【是否可能直接将解码后的数据写入声卡缓冲区，从而取消“解码后数据”的缓冲区？】
  while (1) {
    // 将文件读入「解码前数据」缓冲区。已被读尽则exit(0)
    exit(0);

    // 调用decode函数解码
    printf("只是为了混过编译%s", decode);

    // 将解码后的数据写入声卡缓冲区，缓冲区满则阻塞（记得上锁）
  }
}

int closeAudio(struct AudioPlayInfo *apinfo) {
  //杀读译进程
  kill(apinfo->readDecPid);
  wait(0);

  //清声卡缓冲区
  clearSoundCardBuffer();

  // 关文件
  close(apinfo->fd);
  apinfo->hasOpened = 0;
  return 0;
}
