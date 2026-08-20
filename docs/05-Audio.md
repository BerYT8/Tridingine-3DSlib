# Audio

The **Audio** module provides a platform-independent interface for playing sound effects, music, voices and ambient sounds.

The API is based around the `S3D_*` audio resources and provides the same interface across supported platforms.

Currently supported audio resource types are:

- Sound effects
- Music
- Voice
- Ambience

The audio system also provides:

- Master volume control
- Per-category volume control
- Playback control
- Pause and resume
- Channel selection
- Repetition control
- Platform-independent resource management

---

# Initialization

Before using the audio system, it must be initialized:

```c
s3d_init();
```

The audio system must be initialized before creating or playing audio resources.

When the application exits:

```c
s3d_exit();
```

---

# Frame Updates

The audio system requires a frame update.

At the beginning of each frame, call:

```c
s3d_begin_frame();
```

At the end of the frame, call:

```c
s3d_end_frame();
```

A typical game loop looks like:

```c
s3d_init();

while (S2S_ScreensRunning())
{
    S2S_BeginFrame();
    s3d_begin_frame();

    // Input
    // Game logic
    // Rendering
    // Audio

    s3d_end_frame();
    S2S_EndFrame();
}

s3d_exit();
```

> `s3d_begin_frame()` and `s3d_end_frame()` should be called once per frame.

On Nintendo 3DS, `s3d_begin_frame()` is also responsible for maintaining streaming audio such as Opus audio.

---

# Volume

The audio system provides a master volume and independent volumes for each audio category.

Volume values use a range from `0` to `100`.

```text
0   → Silent
100 → Maximum volume
```

---

# Master Volume

Set the master volume with:

```c
s3d_set_master_volume(100);
```

Get the current master volume:

```c
int volume = s3d_get_master_volume();
```

For example:

```c
s3d_set_master_volume(75);
```

sets the global audio volume to 75%.

The master volume affects the complete audio system.

---

# Sound Volume

Sound effects have their own volume control.

Set the sound-effect volume:

```c
s3d_set_sound_volume(100);
```

Get the current sound-effect volume:

```c
int volume = s3d_get_sound_volume();
```

For example:

```c
s3d_set_sound_volume(50);
```

sets the sound-effect category to 50%.

---

# Music Volume

Music has an independent volume control.

Set the music volume:

```c
s3d_set_music_volume(100);
```

Get the current music volume:

```c
int volume = s3d_get_music_volume();
```

For example:

```c
s3d_set_music_volume(75);
```

sets the music category to 75%.

---

# Voice Volume

Voice resources have their own volume control.

Set the voice volume:

```c
s3d_set_voice_volume(100);
```

Get the current voice volume:

```c
int volume = s3d_get_voice_volume();
```

For example:

```c
s3d_set_voice_volume(80);
```

---

# Ambience Volume

Ambient sounds have their own volume control.

Set the ambience volume:

```c
s3d_set_ambi_volume(100);
```

Get the current ambience volume:

```c
int volume = s3d_get_ambi_volume();
```

For example:

```c
s3d_set_ambi_volume(60);
```

---

# Sound Effects

Sound effects are intended for short audio such as:

- Player actions
- Button sounds
- Collisions
- Weapons
- Enemies
- UI effects
- Gameplay events

Sound effects use the `S3D_Sound` type.

---

# Creating a Sound

Create a sound resource with:

```c
S3D_Sound *sound =
    s3d_make_sound("assets/audio/jump");
```

The path is specified **without the `.opus` extension**.

For example:

```text
assets/audio/jump
```

refers to the corresponding audio resource.

If the sound cannot be created, the function returns `NULL`.

---

# Playing a Sound

Play a sound with:

```c
int result = s3d_play_sound(
    sound,
    100,
    -1,
    1
);
```

The parameters are:

| Parameter | Description |
|-----------|-------------|
| `sound` | Sound resource to play. |
| `volume` | Playback volume from `0` to `100`. |
| `channel` | Channel from `0` to `23`, or a negative value for automatic selection. |
| `repeats` | Number of repetitions. `0` means infinite playback. |

For example:

```c
s3d_play_sound(
    jumpSound,
    100,
    -1,
    1
);
```

This plays the sound once using an automatically selected channel.

---

# Sound Channels

The audio system provides channels numbered:

```text
0 - 23
```

A specific channel can be requested:

```c
s3d_play_sound(
    sound,
    100,
    3,
    1
);
```

This attempts to play the sound on channel `3`.

To allow the system to select an available channel automatically, use a negative channel:

```c
s3d_play_sound(
    sound,
    100,
    -1,
    1
);
```

---

# Sound Repetition

The `repeats` parameter controls how many times the sound is played.

```text
0 → Infinite
1 → Once
2 → Twice
3 → Three times
...
```

For infinite playback:

```c
s3d_play_sound(
    sound,
    100,
    -1,
    0
);
```

---

# Sound Playback Results

`s3d_play_sound()` returns a result code.

Successful playback:

```c
S3D_PLAY_SUCCESS
```

Possible errors include:

```c
S3D_PLAY_ERR_NO_SOUND
S3D_PLAY_ERR_FORMAT
S3D_PLAY_ERR_NO_CHANNEL
S3D_PLAY_ERR_PLATFORM
S3D_PLAY_ERR_PLAYING_SOUND
```

For example:

```c
int result = s3d_play_sound(
    sound,
    100,
    -1,
    1
);

if (result != S3D_PLAY_SUCCESS)
{
    // Playback failed
}
```

The error codes are:

| Code | Meaning |
|------|---------|
| `S3D_PLAY_SUCCESS` | Playback started successfully. |
| `S3D_PLAY_ERR_NO_SOUND` | The sound does not exist or has an invalid type. |
| `S3D_PLAY_ERR_FORMAT` | No supported format exists for the specified sound type. |
| `S3D_PLAY_ERR_NO_CHANNEL` | No channel is currently available. |
| `S3D_PLAY_ERR_PLATFORM` | An unexpected platform error occurred. |
| `S3D_PLAY_ERR_PLAYING_SOUND` | The sound has already been started. |

---

# Pausing a Sound

Pause a currently playing sound with:

```c
s3d_pause_sound(sound);
```

The current playback position is preserved.

The sound can later be continued using:

```c
s3d_continue_sound(sound);
```

Example:

```c
s3d_pause_sound(sound);

// Game paused

s3d_continue_sound(sound);
```

---

# Stopping a Sound

Stop a sound completely with:

```c
s3d_stop_sound(sound);
```

Unlike pausing, stopping cancels the current playback and releases the channel used by the sound.

The `S3D_Sound` resource itself remains valid and can be played again:

```c
s3d_stop_sound(sound);

s3d_play_sound(
    sound,
    100,
    -1,
    1
);
```

---

# Freeing a Sound

When a sound resource is no longer needed, release it with:

```c
s3d_free_sound(sound);
```

If the sound is currently playing, its playback is stopped before the resource is released.

Example:

```c
S3D_Sound *jumpSound =
    s3d_make_sound("assets/audio/jump");

s3d_play_sound(
    jumpSound,
    100,
    -1,
    1
);

// ...

s3d_free_sound(jumpSound);
```

---

# Music

Music is intended for longer audio tracks such as:

- Background music
- Menus
- Gameplay themes
- Boss music
- Ambient tracks

Music resources use the `S3D_Music` type.

---

# Creating Music

Create a music resource with:

```c
S3D_Music *music =
    s3d_make_music("assets/audio/gameplay");
```

The path is specified without the `.opus` extension.

If the resource cannot be created, the function returns `NULL`.

---

# Playing Music

Play music with:

```c
s3d_play_music(
    music,
    100,
    -1,
    0
);
```

A common configuration for background music is:

```c
s3d_play_music(
    gameplayMusic,
    100,
    -1,
    0
);
```

Since `repeats = 0` means infinite playback, the music will continue looping.

---

# Pausing Music

Pause music:

```c
s3d_pause_music(music);
```

Resume it:

```c
s3d_continue_music(music);
```

The playback position is preserved while paused.

---

# Stopping Music

Stop music completely:

```c
s3d_stop_music(music);
```

Stopping releases the channel used for playback while keeping the `S3D_Music` resource valid.

The music can therefore be played again later.

---

# Freeing Music

When music is no longer needed:

```c
s3d_free_music(music);
```

If the music is playing, playback is stopped before its resources are released.

---

# Voices

The Audio API also supports voice resources.

Voices use the `S3D_Voice` type and are useful for:

- Character dialogue
- Narration
- Announcements
- Voice lines
- Spoken instructions

---

# Creating a Voice

Create a voice resource with:

```c
S3D_Voice *voice =
    s3d_make_voice("assets/audio/hello");
```

The path is specified without the `.opus` extension.

---

# Playing a Voice

Play a voice with:

```c
s3d_play_voice(
    voice,
    100,
    -1,
    1
);
```

The parameters work the same way as sound effects:

- Volume from `0` to `100`.
- Channel from `0` to `23`.
- Negative channel for automatic selection.
- `0` repetitions means infinite playback.

---

# Pausing and Continuing Voices

Pause a voice:

```c
s3d_pause_voice(voice);
```

Continue it:

```c
s3d_continue_voice(voice);
```

Stop it completely:

```c
s3d_stop_voice(voice);
```

---

# Freeing a Voice

Release a voice resource with:

```c
s3d_free_voice(voice);
```

---

# Ambience

Ambient audio is represented by the `S3D_Ambi` type.

It can be used for sounds such as:

- Wind
- Rain
- Forest ambience
- Environmental loops
- Background environmental sounds

---

# Creating Ambience

Create an ambient resource with:

```c
S3D_Ambi *ambi =
    s3d_make_ambi("assets/audio/forest");
```

The path is specified without the `.opus` extension.

---

# Playing Ambience

Play an ambient sound with:

```c
s3d_play_ambi(
    ambi,
    100,
    -1,
    0
);
```

Using `0` repetitions causes the ambience to loop indefinitely.

---

# Pausing and Continuing Ambience

Pause:

```c
s3d_pause_ambi(ambi);
```

Continue:

```c
s3d_continue_ambi(ambi);
```

Stop:

```c
s3d_stop_ambi(ambi);
```

---

# Freeing Ambience

When the ambient resource is no longer required:

```c
s3d_free_ambi(ambi);
```

---

# Resource Lifecycle

Each audio resource follows the same general lifecycle:

```text
Initialize Audio
      │
      ▼
Create Resource
      │
      ▼
Play Resource
      │
      ├──── Pause
      │       │
      │       ▼
      │    Continue
      │
      ▼
Stop Resource
      │
      ▼
Free Resource
      │
      ▼
Exit Audio
```

Resources should normally be created once and reused instead of repeatedly loading the same audio during gameplay.

---

# Complete Example

The following example demonstrates sound effects and looping music:

```c
#include <Tridingine.h>

int app_main()
{
    S2S_ScreensInit();

    s3d_init();

    S3D_Sound *jumpSound =
        s3d_make_sound("assets/audio/jump");

    S3D_Music *music =
        s3d_make_music("assets/audio/gameplay");

    s3d_set_master_volume(100);
    s3d_set_sound_volume(100);
    s3d_set_music_volume(75);

    s3d_play_music(
        music,
        100,
        -1,
        0
    );

    while (S2S_ScreensRunning())
    {
        S2S_BeginFrame();

        s3d_begin_frame();

        input_read();

        if (input_isKeyPressed(INPUT_KEY_A))
        {
            s3d_play_sound(
                jumpSound,
                100,
                -1,
                1
            );
        }

        // Game logic
        // Rendering

        s3d_end_frame();

        S2S_EndFrame();
    }

    s3d_stop_music(music);

    s3d_free_sound(jumpSound);
    s3d_free_music(music);

    s3d_exit();

    S2S_ScreensExit();

    return 0;
}
```

---

# Platform Independence

The Audio API is designed to keep game code independent from the underlying platform audio implementation.

Game code interacts with the `s3d_*` API:

```c
s3d_play_sound(sound, 100, -1, 1);
```

instead of directly accessing platform-specific audio systems.

This allows the same game code to use audio on the supported platforms.

Nintendo 3DS-specific audio handling is kept inside the audio implementation.

---

# Development Status

The **Audio** API is currently **usable**.

The API supports:

- Sound effects
- Music
- Voices
- Ambience
- Volume control
- Playback control
- Pause and resume
- Channel selection
- Repetition

The API may still receive additional functionality in future versions, but the current interface can be used by projects targeting the supported platforms.

---

# Next Step

Now that you know how to initialize the audio system and work with sound effects, music, voices and ambience, the next step is learning how to manage other engine resources.

Continue with [**06 - Resources**](06-Resources.md), where you will learn how to:

- Work with engine resources.
- Load and release resources.
- Organize resources used by your game.
- Manage resources across your project.