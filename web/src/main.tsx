import React, { useEffect, useState } from "react";
import { createRoot } from "react-dom/client";
import "./style.css";
type Message = {version:number;type:string;sequence:number;pressed?:string[];data?:{code:string;action:string}};
function Overlay(){
 const [keys,setKeys]=useState<string[]>([]);
 const [connected,setConnected]=useState(false);
 useEffect(()=>{
  let active=true;
  let socket:WebSocket|undefined;
  let retry:ReturnType<typeof setTimeout>|undefined;
  function connect(){
   if(!active)return;
   const url=new URL("/ws/v1/overlay/default",window.location.href);
   url.protocol=url.protocol==="https:"?"wss:":"ws:";
   socket=new WebSocket(url);
   socket.onopen=()=>setConnected(true);
   socket.onmessage=(event)=>{
    try{
     const message=JSON.parse(event.data) as Message;
     if(message.version!==1)return;
     if(message.type==="state.snapshot")setKeys(message.pressed??[]);
     if(message.type==="input.key"&&message.data){
      const {code,action}=message.data;
      setKeys(prev=>action==="down"?[...new Set([...prev,code])]:prev.filter(x=>x!==code));
     }
    }catch{/* ignore invalid messages */}
   };
   socket.onclose=()=>{setConnected(false);setKeys([]);if(active)retry=setTimeout(connect,1000)};
   socket.onerror=()=>socket?.close();
  }
  connect();
  return()=>{active=false;clearTimeout(retry);socket?.close()};
 },[]);
 return <main className="overlay" aria-label="Live Keycast overlay">{keys.map(key=><span className="key" key={key}>{key}</span>)}{!connected&&<span className="connection">Waiting for Keycast…</span>}</main>;
}
function App(){
 return window.location.pathname.startsWith("/overlay")?<Overlay/>:<main className="dashboard"><h1>Keycast</h1><p>Run <code>go run ./cmd/keycast -simulate</code> and <code>cd web && npm run dev</code>.</p><a href="/overlay/default">Open live overlay</a></main>;
}
createRoot(document.getElementById("root")!).render(<React.StrictMode><App/></React.StrictMode>);
