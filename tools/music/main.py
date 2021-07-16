import serial
import sys
import time
import pygame
import re

pygame.init()
pygame.mixer.init()

class Player:
    def __init__(self):
        self.music = {}
    
    def load(self, fileName: str, name: str = ""):
        if not name:
            name = fileName
        self.music[name] = pygame.mixer.Sound(fileName)
        return self.music[name]

    def play(self, name: str):
        self.music[name].play()
        return self.music[name]
    
    def find(self, name: str):
        return self.music[name]

def main():
    serialPort = sys.argv[1] if len(sys.argv) > 1 else "COM6"
    print(f'port: {serialPort}')
    while True:
        try:
            print(f"try connecting port: {serialPort}")
            ser = serial.Serial(serialPort, 128000, timeout=1)
            break
        except:
            time.sleep(1)
            continue
    print("connect success!")
    ser.flushInput()
    currentBgm = "bgm0"

    player = Player()
    player.load("drum-hitclap.wav", "hitclap")
    player.load("drum-hitnormal.wav", "hitnormal")
    # player.load("BadApple.wav", "bad apple").set_volume(0.2)
    player.load("bgm128.mp3", "bad apple").set_volume(0.5)

    player.load("perfect.mp3", "perfect")
    player.load("great.mp3", "great")
    player.load("good.mp3", "good")
    player.load("miss.wav", "miss")
    # player.play("bad apple")
    for i in range(3):
        player.load(f'songs/bgm{i}.mp3', f'bgm{i}').set_volume(0.5)

    msg_buf = ""
    while True:
        if ser.inWaiting() != 0:
            msg_buf += ser.read(ser.in_waiting).decode("utf8").replace("\n", "")
        idx = msg_buf.find(";")
        if idx == -1:
            time.sleep(0.01)
            continue
        msg = msg_buf[:idx]
        msg_buf = msg_buf[idx+1:]

        if msg == "hitclap":
            player.play("hitclap")
        elif msg == "hitnormal":
            player.play("hitnormal")
        elif msg == "bad apple":
            player.play("bad apple")
        elif msg == "reset":
            pygame.mixer.stop()
        elif msg == "perfect":
            player.play("perfect")
        elif msg == "great":
            player.play("great")
        elif msg == "good":
            player.play("good")
        elif msg == "miss":
            player.play("miss")
        elif re.findall("bgm[0-9]+", msg):
            msg = re.findall("bgm[0-9]+", msg)[0]
            player.find(currentBgm).stop()
            currentBgm = msg
            player.play(msg.strip('\n'))
        print(msg)
        

if __name__ == '__main__':
    main()