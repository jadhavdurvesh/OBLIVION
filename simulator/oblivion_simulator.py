#!/usr/bin/env python3
"""OBLIVION standalone protocol simulator. No physical motors or hardware access."""
from __future__ import annotations
import argparse, math, socket, struct, threading, time

MAGIC=b"OB"; VERSION=1; TELEMETRY=6; MAX_PAYLOAD=512
VALID=(1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)

def crc16(data: bytes) -> int:
    crc=0xffff
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc=((crc<<1)^0x1021)&0xffff if crc&0x8000 else (crc<<1)&0xffff
    return crc

def frame(kind: int, seq: int, payload: bytes=b"") -> bytes:
    head=MAGIC+bytes((VERSION,kind))+struct.pack("<IH",seq,len(payload))+payload
    return head+struct.pack("<H",crc16(head))

def unpack_frame(data: bytes):
    if len(data)<12 or data[:2]!=MAGIC or data[2]!=VERSION: return None
    size=struct.unpack_from("<H",data,8)[0]
    if size>MAX_PAYLOAD or len(data)!=12+size: return None
    if crc16(data[:-2])!=struct.unpack_from("<H",data,len(data)-2)[0]: return None
    return data[3],struct.unpack_from("<I",data,4)[0],data[10:-2]

class Robot:
    def __init__(self):
        self.x=0.0; self.y=0.0; self.heading=0.0; self.speed=0.0
        self.active=False; self.profile=0; self.mask=0x023f; self.seq=0
        self.left_ticks=0; self.right_ticks=0; self.started=time.monotonic()
    def command(self,kind,payload):
        if kind==4: self.active=False; self.speed=0.0; return
        if kind==2 and len(payload)>=4:
            action,profile,lo,hi=payload[:4]; self.profile=profile; self.mask=lo|(hi<<8)
            if action==1: self.active=True
            elif action in (0,2): self.active=False; self.speed=0.0
    def telemetry(self):
        now=time.monotonic(); t=now-self.started
        target=0.85 if self.active else 0.0
        self.speed += max(-2.0,min(2.0,target-self.speed))*0.025
        curve=0.65*math.sin(t*0.48) if self.profile==0 else 0.2*math.sin(t*0.18)
        self.heading += curve*self.speed*0.025
        self.x += math.cos(self.heading)*self.speed*0.025
        self.y += math.sin(self.heading)*self.speed*0.025
        self.left_ticks += int((self.speed-curve*0.06)*100)
        self.right_ticks += int((self.speed+curve*0.06)*100)
        position=int(3200*math.sin(t*0.48)) if self.active else 0
        conf=970 if self.active else 1000
        sensors=[int(max(0,min(1000,950-780*math.exp(-((i-3.5-position/2500)**2)/1.6)))) for i in range(8)]
        yaw=int(curve*self.speed*1000)
        ctrl=[]
        for i in range(11):
            enabled=(self.mask>>i)&1
            ctrl.append((int(max(-1000,min(1000,-position/4))),int(110+i*7) if enabled else 0,1 if enabled else 0))
        payload=bytearray()
        payload+=struct.pack("<QI8H iH ii hhh hhh iii H",int(t*1e6),VALID,*sensors,position,conf,self.left_ticks,self.right_ticks,0,0,yaw,0,0,1000,self.speed*1000,yaw,int(curve*1000),self.mask)
        for o,us,status in ctrl: payload+=struct.pack("<hHB",o,us,status)
        payload+=struct.pack("<BIHI",18,65536,7400,0)
        return bytes(payload)

def client(conn,addr,stop):
    robot=Robot(); conn.settimeout(.05); buf=bytearray()
    try:
        while not stop.is_set():
            try:
                chunk=conn.recv(4096)
                if not chunk: break
                buf.extend(chunk)
            except socket.timeout: pass
            while len(buf)>=12:
                if buf[:2]!=MAGIC: del buf[0]; continue
                n=struct.unpack_from("<H",buf,8)[0]
                if n>MAX_PAYLOAD: del buf[0]; continue
                total=12+n
                if len(buf)<total: break
                packet=unpack_frame(bytes(buf[:total])); del buf[:total]
                if packet: robot.command(packet[0],packet[2])
            robot.seq+=1
            conn.sendall(frame(TELEMETRY,robot.seq,robot.telemetry()))
            time.sleep(0.025)
    except (ConnectionError,OSError): pass
    finally: conn.close()

def main():
    parser=argparse.ArgumentParser(description="OBLIVION virtual robot telemetry endpoint")
    parser.add_argument("--host",default="127.0.0.1"); parser.add_argument("--port",type=int,default=8765)
    args=parser.parse_args(); stop=threading.Event()
    with socket.socket() as server:
        server.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1); server.bind((args.host,args.port)); server.listen()
        print(f"OBLIVION SIMULATOR READY · TCP {args.host}:{args.port} · Ctrl+C to stop",flush=True)
        try:
            while True:
                conn,addr=server.accept()
                threading.Thread(target=client,args=(conn,addr,stop),daemon=True).start()
        except KeyboardInterrupt: stop.set()
if __name__=="__main__": main()
