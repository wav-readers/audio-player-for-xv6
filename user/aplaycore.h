// audio player core 音频播放库

//简便起见，假定声卡一次只会播放一个音频。
//故暂停、调音量等操作直接干预声卡，这样就只需要控制器和读译两个进程。

#ifndef USER_APLAYCORE_H
#define USER_APLAYCORE_H

#include "user/wav.h"

// mp3相关内容以"未实现"标记。没写todo因为可能无余力实现
enum ApFileType { WAV, MP3 };

struct ApAudioPlayInfo {
  // 始终可用
  int hasOpened;
  int maxVolume;
  int volume;  // 全局音量

  // 以下只在 hasOpened == 1 时可用
  // 音频文件信息
  const char *fname;
  enum ApFileType ftype;
  union {
    struct WavInfo wavInfo;
  };
  // 播放状态
  int isPlaying;
  // 后台信息
  int fd;
  int readDecPid;
};
struct ApAudioPlayInfo *ApAudioPlayInfo();

/**
 * @pre apinfo->hasOpened == 0
 * @brief 打开音频文件并检查格式。含报错提示。
 * @return 音频文件的fd if 成功 else -1
 */
int apOpenAudio(const char *file, struct ApAudioPlayInfo *apinfo);

/// @pre apinfo->hasOpened == 1
void apShowAudioInfo(struct ApAudioPlayInfo *apinfo);

/// @return 新建进程的pid if 成功 else -1
int apReadDecode(struct ApAudioPlayInfo *apinfo);

/**
 * @pre apinfo->hasOpened == 1
 * @details 杀读译进程+清声卡缓冲区+关文件
 * @return 0 if 成功 else -1
 */
int apCloseAudio(struct ApAudioPlayInfo *apinfo);

/**
 * @pre apinfo->hasOpened == 1
 * @brief 设置播放状态
 * @param play 0为暂停，1为播放
 * @return 0 if 成功 else -1
 */
int apSetPlay(int play, struct ApAudioPlayInfo *apinfo);

/// @return 原音量 if 成功 else -1
int apSetVolume(int volume, struct ApAudioPlayInfo *apinfo);

void apSetMaxVolume(int maxVolume, struct ApAudioPlayInfo *apinfo);

#endif