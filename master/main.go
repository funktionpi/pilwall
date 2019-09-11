package main

import (
  "fmt"
  "github.com/micro/mdns"
  "image/color"
  "led-wall/pkg/errors"
  "led-wall/pkg/slave"
  "net/url"
  "os"
  "os/signal"
  "sync"
  "time"
)

func main() {

  entriesCh := make(chan *mdns.ServiceEntry, 4)
  // Start the lookup
  err := mdns.Lookup("_ledwall._tcp", entriesCh)

  errors.ExitIfErr(err)
  close(entriesCh)


  var svr *mdns.ServiceEntry
  for entry := range entriesCh {
    fmt.Printf("Service '%v' - %v (%v) - %v:%v\n", entry.Name, entry.Info, entry.Host, entry.AddrV4, entry.Port)
    svr = entry
    break
  }

  if svr == nil {
    println("no ledslave found on network")
    os.Exit(1)
  }

  //addr := fmt.Sprintf("ws://%s:%d", svr.Addr.String(), svr.Port)
  u := url.URL{
    Scheme: "ws",
    Host: fmt.Sprintf("%s:%d", svr.Addr.String(), svr.Port),
    Path: "/ws",
  }

  client, err := slave.Connect(u)
  errors.ExitIfErr(err)
  defer client.Close()

  client.SetBrightness(8)

  colors := []color.RGBA{
    { R: 255 },
    { G: 255 },
    { B: 255 },
  }

  it := 0
  for {
    err = client.Clear(colors[it])
    it = (it + 1)%len(colors)
    errors.ExitIfErr(err)
    time.Sleep(time.Millisecond * 150)
  }


  //WaitForCtrlC()
}


func WaitForCtrlC() {
  var end_waiter sync.WaitGroup
  end_waiter.Add(1)
  var signal_channel chan os.Signal
  signal_channel = make(chan os.Signal, 1)
  signal.Notify(signal_channel, os.Interrupt)
  go func() {
    <-signal_channel
    end_waiter.Done()
  }()
  end_waiter.Wait()
}
