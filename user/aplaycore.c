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
  apinfo->speed = 1.0;
  return apinfo;
}

void apSetMaxVolume(int maxVolume, struct ApAudioPlayInfo *apinfo) {
  apinfo->maxVolume = maxVolume;
}

void updateSampleRate(struct ApAudioPlayInfo *apinfo) {
  setSampleRate((double)apinfo->ainfo.sample_rate * apinfo->speed *
                (double)apinfo->ainfo.num_channels / 2.0);
  if (apinfo->isPlaying) {
    setPlay(0);
    setPlay(1);
  }
}

int apSetPlay(int play, struct ApAudioPlayInfo *apinfo) {
  // printf("apSetPlay, play = %d\n", play);
  if (play == apinfo->isPlaying) {  // 无需修改，则直接返回，以减少开销
    return 0;
  }
  setPlay(play);
  apinfo->isPlaying = play;
  return 0;
}

int apSetVolume(int volume, struct ApAudioPlayInfo *apinfo) {
  if (!(volume >= 0 && volume <= apinfo->maxVolume)) {
    fprintf(2, "invalid volume value\n");
    return -1;
  }

  setVolume(volume, apinfo->maxVolume);
  int actualVolume = getVolume(apinfo->maxVolume);
  apinfo->volume = actualVolume;
  return 0;
}

int apSetSpeed(double speed, struct ApAudioPlayInfo *apinfo) {
  if (speed <= 0.0) {
    fprintf(2, "invalid speed value\n");
    return -1;
  }
  int actualSampleRate = (double)apinfo->ainfo.sample_rate * speed *
                         (double)apinfo->ainfo.num_channels / 2.0;
  if (actualSampleRate > 65535) {
    fprintf(2, "too high speed value for this audio\n");
    return -1;
  }

  apinfo->speed = speed;
  updateSampleRate(apinfo);
  return 0;
}

int apOpenAudio(const char *file, struct ApAudioPlayInfo *apinfo) {
  //printf("apOpenAudio\n");

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
      if (readWavHead(fd, &apinfo->ainfo, &apinfo->wavInfo) < 0) {
        fprintf(2, "invalid file format\n");
        close(fd);
        return -1;
      }
      break;
    }
    case MP3:
      //未实现
      break;
  }
  clearSoundCardBuffer();
  apinfo->hasOpened = 1;
  // 填写其他信息：音频文件、播放状态、后台信息
  apinfo->fname = file;
  apinfo->isPlaying = 0;
  apinfo->speed = 1.0;
  apinfo->volume = getVolume(apinfo->maxVolume);
  apinfo->fd = fd;
  apinfo->readDecPid = -1;
  updateSampleRate(apinfo);
  return fd;
}

void apShowAudioInfo(struct ApAudioPlayInfo *apinfo) {
  printf("file name: %s\n", apinfo->fname);
  printf("sample rate: %d\n", apinfo->ainfo.sample_rate);
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
      if (nRead == 0) {
        finishwriteaudio();
        exit(0);
      }
      writeDecodedAudio(fileData, nRead);
    }
  }
  /* 处理其他音频格式
    #define DEC_BUFFER_SIZE 2048 // 应根据比特率估算所需缓冲区大小
    void (*decode)(const char *fileData, char *decodedData) = 0;
    switch (apinfo->ftype) {
      case MP3:
        decode = decodeMp3;
        break;
    }
    char decodedData[DEC_BUFFER_SIZE];
    while (1) {
      int nRead = read(fd, fileData, READ_BUFFER_SIZE);
      if (nRead == 0) {
        finishwriteaudio();
        exit(0);
      }
      int nDec = decode(fileData, decodedData);
      writeDecodedAudio(decodedData, nDec);
    }
  */
  exit(0);
}

int apCloseAudio(struct ApAudioPlayInfo *apinfo) {
  //printf("apCloseAudio\n");
  //杀读译进程
  kill(apinfo->readDecPid);

  //清声卡缓冲区
  clearSoundCardBuffer();

  // 关文件
  close(apinfo->fd);
  apinfo->hasOpened = 0;
  return 0;
}
