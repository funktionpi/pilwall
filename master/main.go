package main

import (
  "fmt"
  "github.com/micro/mdns"
  "led-wall/pkg/errors"
  "os"
  "os/signal"
  "sync"
)

func main() {

  entriesCh := make(chan *mdns.ServiceEntry, 4)
  go func() {
    for entry := range entriesCh {
      fmt.Printf("Service '%v' - %v (%v) - %v:%v\n", entry.Name, entry.Info, entry.Host, entry.AddrV4, entry.Port)
    }
  }()
  // Start the lookup
  err := mdns.Lookup("_ledwall._tcp", entriesCh)
  errors.ExitIfErr(err)
  close(entriesCh)

  WaitForCtrlC()
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
