// audio player core 音频播放库

//简便起见，假定声卡一次只会播放一个音频。
//故暂停、调音量等操作直接干预声卡，这样就只需要控制器和读译两个进程。

#ifndef USER_APLAYCORE_H
#define USER_APLAYCORE_H

#include "user/wav.h"

// mp3相关内容以"未实现"标记。没写todo因为可能无余力实现
enum FileType { WAV, MP3 };

struct AudioPlayInfo {
  // 始终可用
  int hasOpened;

  // hasOpened == 1
  // 音频文件信息
  const char *fname;
  enum FileType ftype;
  union {
    struct WavInfo wavInfo;
  };
  // 播放状态
  int isPlaying;
  int volume;
  // 后台信息
  int fd;
  int readDecPid;
};
struct AudioPlayInfo *AudioPlayInfo();

/**
 * @pre apinfo->hasOpened == 0
 * @brief 打开音频文件并检查格式。含报错提示。
 * @return 音频文件的fd if 成功 else -1
 */
int openAudio(const char *file, struct AudioPlayInfo *apinfo);

/// @pre apinfo->hasOpened == 1
void showAudioInfo(struct AudioPlayInfo *apinfo);

/// @return 新建进程的pid if 成功 else -1
int beginReadDecode(struct AudioPlayInfo *apinfo);

/**
 * @pre apinfo->hasOpened == 1
 * @details 杀读译进程+清声卡缓冲区+关文件
 * @return 0 if 成功 else -1
 */
int closeAudio(struct AudioPlayInfo *apinfo);

/**
 * @brief 设置播放状态
 * @param play 0为暂停，1为播放
 * @return 0 if 成功 else -1
 */
int setPlay(int play, struct AudioPlayInfo *apinfo);

/// @return 原音量 if 成功 else -1
int setVolume(int volume, struct AudioPlayInfo *apinfo);

#endif