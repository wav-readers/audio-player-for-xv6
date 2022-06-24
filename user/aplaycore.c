#include "aplaycore.h"

#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/wav.h"

int getVolume() {
  // 做成系统调用，无需在此库中提供对外接口
  // 应用程序可从apinfo中读取volume信息
  return 0;
}

struct AudioPlayInfo *AudioPlayInfo() {
  struct AudioPlayInfo *apinfo =
      (struct AudioPlayInfo *)malloc(sizeof(struct AudioPlayInfo));
  apinfo->hasOpened = 0;
  apinfo->volume = getVolume();
  return apinfo;
}

///@todo
// 做成系统调用，无需在此库中提供对外接口
int clearSoundCardBuffer() { return 0; }
// 做成系统调用，无需在此库中提供对外接口
int setSampleRate(uint sample_rate) { return 0; }
/// 以上

///@todo 以下在系统调用的基础上套层壳，使成为库函数而非系统调用
int setPlay(int play, struct AudioPlayInfo *apinfo) {
  // 调用系统调用
  return 0;
}

int setVolume(int volume, struct AudioPlayInfo *apinfo) {
  // 调用系统调用
  return volume;
}
/// 以上

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
      if (readWavHead(fd, &apinfo->wavInfo) < 0) {
        fprintf(2, "invalid file format");
        close(fd);
        return -1;
      }
      setSampleRate(apinfo->wavInfo.sample_rate);
      break;
    }
    case MP3:
      //未实现
      break;
  }

  apinfo->hasOpened = 1;
  // 填写其他信息：音频文件、播放状态、后台信息
  apinfo->fname = file;
  apinfo->isPlaying = 0;
  apinfo->volume = getVolume();
  apinfo->fd = fd;
  apinfo->readDecPid = -1;
  return fd;
}

void showAudioInfo(struct AudioPlayInfo *apinfo) {
  printf("file name: %s\n", apinfo->fname);
  switch (apinfo->ftype) {
    case WAV:
      printf("sample rate: %d\n", apinfo->wavInfo.sample_rate);
      break;
    case MP3:
      ///@todo
      break;
  }
  printf("volume: %d\n", apinfo->volume);
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
  // question: 缓冲区大小?
  while (1) {
    // 将文件读入「解码前数据」缓冲区。已被读尽则exit(0)
    exit(0);

    // 调用decode函数解码
    printf("只是为了混过编译%s", decode);

    // 调用系统调用，将解码后的数据写入声卡缓冲区
    // 若可能卡顿，可考虑优化缓存机制？
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
