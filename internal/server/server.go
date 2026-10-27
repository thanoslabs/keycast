package server

import (
 "crypto/sha1"
 "encoding/base64"
 "encoding/json"
 "fmt"
 "net"
 "net/http"
 "strings"
 "sync"
 "time"
 "github.com/thanoslabs/keycast/internal/events"
 "github.com/thanoslabs/keycast/internal/state"
)
const magic="258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
type Hub struct { mu sync.Mutex; store *state.Store; clients map[chan []byte]struct{} }
func New()*Hub{return &Hub{store:state.New(),clients:make(map[chan []byte]struct{})}}
func(h *Hub)Emit(key events.KeyEvent){
 h.mu.Lock();defer h.mu.Unlock()
 event,ok:=h.store.Apply(key);if !ok{return}
 data,_:=json.Marshal(event)
 for c:=range h.clients {select {case c<-data:default:close(c);delete(h.clients,c)}}
}
func(h *Hub)Routes()http.Handler{
 mux:=http.NewServeMux()
 mux.HandleFunc("GET /api/v1/health",func(w http.ResponseWriter,_ *http.Request){w.Header().Set("Content-Type","application/json");_,_=w.Write([]byte(`{"status":"ok"}`))})
 mux.HandleFunc("GET /ws/v1/overlay/default",h.overlay)
 return mux
}
func(h *Hub)overlay(w http.ResponseWriter,r *http.Request){
 origin:=r.Header.Get("Origin")
 if origin!="" && origin!="http://"+r.Host {http.Error(w,"invalid origin",http.StatusForbidden);return}
 if !strings.EqualFold(r.Header.Get("Upgrade"),"websocket") || !strings.Contains(strings.ToLower(r.Header.Get("Connection")),"upgrade") || r.Header.Get("Sec-WebSocket-Version")!="13" {http.Error(w,"websocket upgrade required",426);return}
 key:=r.Header.Get("Sec-WebSocket-Key")
 decoded,err:=base64.StdEncoding.DecodeString(key);if err!=nil||len(decoded)!=16{http.Error(w,"invalid key",400);return}
 hj,ok:=w.(http.Hijacker);if !ok{http.Error(w,"hijack unavailable",500);return}
 conn,rw,err:=hj.Hijack();if err!=nil{return};defer conn.Close()
 hash:=sha1.Sum([]byte(key+magic))
 if _,err=fmt.Fprintf(rw,"HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n",base64.StdEncoding.EncodeToString(hash[:]));err!=nil{return}
 if rw.Flush()!=nil{return}
 c:=make(chan []byte,32)
 h.mu.Lock()
 seq,keys:=h.store.Snapshot()
 snapshot,_:=json.Marshal(struct{Version int `json:"version"`;Type string `json:"type"`;Sequence uint64 `json:"sequence"`;Pressed []string `json:"pressed"`}{1,"state.snapshot",seq,keys})
 h.clients[c]=struct{}{}
 h.mu.Unlock()
 defer func(){h.mu.Lock();if _,ok:=h.clients[c];ok{delete(h.clients,c);close(c)};h.mu.Unlock()}()
 if writeFrame(conn,snapshot)!=nil{return}
 for data:=range c {if writeFrame(conn,data)!=nil{return}}
}
func writeFrame(conn net.Conn,data []byte)error{
 if err:=conn.SetWriteDeadline(time.Now().Add(5*time.Second));err!=nil{return err}
 head:=[]byte{0x81}
 if len(data)<126{head=append(head,byte(len(data)))}else{head=append(head,126,byte(len(data)>>8),byte(len(data)))}
 _,err:=conn.Write(append(head,data...));return err
}
