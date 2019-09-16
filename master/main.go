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

var (
  White = color.RGBA{255,255,255,255}
  Black = color.RGBA{}
  Red = color.RGBA{R:255}
  Blue = color.RGBA{B:255}
  Green = color.RGBA{G:255}
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

  err = client.SetBrightness(8)
  errors.ExitIfErr(err)

  dimension, err := client.GetDimension()
  errors.ExitIfErr(err)

  fmt.Printf("screen dimension is %dx%d",dimension.Width, dimension.Height)

  scrobe(client)
  //scanLines(client, dimension)

  //WaitForCtrlC()
}

func scanLines(client *slave.Client, dimension *slave.DimensionResponse) {
  colors := []color.RGBA{
    { R: 255 },
    { G: 255 },
    { B: 255 },
  }

  horizontal := true
  it := int16(0);
  col := 0;

  white := color.RGBA{255,255,255,255}

  for  {
    err := client.Clear(colors[col])
    errors.PrintIfErr(err)

    count := dimension.Width
    if !horizontal {
      count = dimension.Height
    }

    if horizontal {
      err = client.DrawLine(it, 0, it, int16(dimension.Height), white)
      errors.PrintIfErr(err)
    } else {
      err = client.DrawLine(0, it, int16(dimension.Width), it, white)
      errors.PrintIfErr(err)
    }

    it++;

    if it >= int16(count) {
      horizontal = !horizontal;
      it = 0;
      col = (col + 1) % len(colors);
    }
  }
}

func scrobe(client *slave.Client) {
  colors := []color.RGBA{Black, White}

  idx := 0
  for {
    idx = (idx+1) % len(colors)
    client.Clear(colors[idx])
    time.Sleep(time.Millisecond * 40)
  }
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
