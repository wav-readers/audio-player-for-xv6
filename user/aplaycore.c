#include "aplaycore.h"

#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/wav.h"

struct ApAudioPlayInfo *ApAudioPlayInfo() {
  struct ApAudioPlayInfo *apinfo =
      (struct ApAudioPlayInfo *)malloc(sizeof(struct ApAudioPlayInfo));
  apinfo->hasOpened = 0;
  apinfo->maxVolume = 100;
  apinfo->volume = getVolume(apinfo->maxVolume);
  return apinfo;
}

void apSetMaxVolume(int maxVolume, struct ApAudioPlayInfo *apinfo) {
  apinfo->maxVolume = maxVolume;
}

int apSetPlay(int play, struct ApAudioPlayInfo *apinfo) {
  if (play == apinfo->isPlaying) {  // 无需修改，则直接返回，以减少开销
    return 0;
  }
  setPlay(play);
  apinfo->isPlaying = play;
  return 0;
}

int apSetVolume(int volume, struct ApAudioPlayInfo *apinfo) {
  setVolume(volume, apinfo->maxVolume);
  return volume;
}

int apOpenAudio(const char *file, struct ApAudioPlayInfo *apinfo) {
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
  apinfo->volume = getVolume(apinfo->maxVolume);
  apinfo->fd = fd;
  apinfo->readDecPid = -1;
  return fd;
}

void apShowAudioInfo(struct ApAudioPlayInfo *apinfo) {
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

int apReadDecode(struct ApAudioPlayInfo *apinfo) {
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
  int fd = apinfo->fd;
  char fileData[READ_BUFFER_SIZE];
  if (apinfo->ftype == WAV) {
    while (1) {
      int nRead = read(fd, fileData, READ_BUFFER_SIZE);
      if (nRead == 0) exit(0);
      writeDecodedAudio(fileData, nRead);
    }
  }
  /* 处理其他音频格式
    void (*decode)(const char *fileData, char *decodedData) = 0;
    switch (apinfo->ftype) {
      case MP3:
        decode = decodeMp3;
        break;
    }
  #define DEC_BUFFER_SIZE 100
    char decodedData[DEC_BUFFER_SIZE];
    while (1) {
      int nRead = read(fd, fileData, READ_BUFFER_SIZE);
      if (nRead == 0) exit(0);
      int nDec = decode(fileData, decodedData);
      writeDecodedAudio(decodedData, nDec);
    } */
    
}

int apCloseAudio(struct ApAudioPlayInfo *apinfo) {
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
