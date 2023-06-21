import processing.net.*;
import controlP5.*;

Server Server1, Server2, Server3;
Client Client1, Client2, Client3;
ControlP5 p5;
Textarea textArea1, textArea2, textArea3, textArea4;
Boolean ButtonStatus = false, ButtonStatus2 = false;
String ClientMsg1 = "A", ClientMsg2 = "B", ClientMsg3 = "C";
PImage img;

void setup() {
  size(1600, 800);
  img = loadImage("UI3.png");
  Server1 = new Server(this, 5203);
  Server2 = new Server(this, 5204);
  Server3 = new Server(this, 5205);
  noStroke();
  p5 = new ControlP5(this);
  
  PFont font =createFont("FreeSans", 20);
  
  Button StartButton = p5.addButton("Start");
  StartButton.setPosition(int(973), int(133));
  StartButton.setSize(int(150), int(75));
  StartButton.activateBy(ControlP5.PRESS);
  StartButton.setFont(font);
  
  
  Button StopButton = p5.addButton("Stop");
  StopButton.setPosition(int(1333), int(133));
  StopButton.setSize(int(150), int(75));
  StopButton.activateBy(ControlP5.PRESS);
  StopButton.setFont(font);
  
  Button ClearButton = p5.addButton("Clear");
  ClearButton.setPosition(int(1150), int(450));
  ClearButton.setSize(int(150), int(75));
  ClearButton.activateBy(ControlP5.PRESS);
  ClearButton.setFont(font);
  

  textArea1 = p5.addTextarea("txt")
  //.setPosition(int(0.2 * width), int(0.4 * height))
  .setPosition(50, 450)
  //.setSize(int(0.85 * width), int(0.2 * height))
  .setSize(700, 250)
  .setColor(color(20))
  .setFont(font);
  
  textArea2 = p5.addTextarea("txt2")
  //.setPosition(int(0.2 * width), int(0.4 * height))
  .setPosition(50, 100)
  //.setSize(int(0.85 * width), int(0.2 * height))
  .setSize(300, 250)
  .setColor(color(20))
  .setFont(font);
  
  textArea3 = p5.addTextarea("txt3")
  //.setPosition(int(0.2 * width), int(0.4 * height))
  .setPosition(450, 100)
  //.setSize(int(0.85 * width), int(0.2 * height))
  .setSize(300, 250)
  .setColor(color(20))
  .setFont(font);
  

  
  
}

void draw() {
  image(img, 0, 0);
  
  
  Client Client1 = Server1.available();
  if ( Client1 != null) {
    ClientMsg1 = Client1.readString();
    //String ClientMsg1 = String(ClientMsg1);
    textArea1.append(ClientMsg1);
    textArea1.append("\n");
  }
  
   Client Client2 = Server2.available();
   if ( Client2 != null){
     ClientMsg2 = Client2.readString();
     //String ClientMsg2 = ClientMsg2;
     textArea2.append(ClientMsg2);
     textArea2.append("\n");
   }
   
   Client Client3 = Server3.available();
  if ( Client1 != null) {
    ClientMsg3 = Client3.readString();
    //String ClientMsg3 = ClientMsg3;
    textArea3.append(ClientMsg3);
    textArea3.append("\n");
    if ( int(ClientMsg3) >= 25){
      textArea1.append("Approaching Critical Slope");
      textArea1.append("\n");
  }


      }
    
}



public void Start() {
  if (ButtonStatus == false) {
     Server1.write('w');
     ButtonStatus = true;
     textArea1.append("Buggy engaged\n");
  }
}

public void Stop() {
  if (ButtonStatus == true) {
    Server1.write('s');
    ButtonStatus = false;
    textArea1.append("Buggy disengaged\n");
  }
}

public void Clear() {
  textArea1.clear();
}
