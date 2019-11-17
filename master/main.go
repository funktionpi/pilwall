package main

import (
	"context"
	"fmt"
	"github.com/grandcat/zeroconf"
	"github.com/micro/mdns"
	"image/color"
	"led-wall/pkg/errors"
	"led-wall/pkg/slave"
	"log"
	"net"
	"net/url"
	"os"
	"os/signal"
	"strings"
	"sync"
	"time"
)

var (
	White = color.RGBA{255, 255, 255, 255}
	Black = color.RGBA{}
	Red   = color.RGBA{R: 255}
	Blue  = color.RGBA{B: 255}
	Green = color.RGBA{G: 255}
)

func main() {

	println("looking for mDNS services... ")

	var dnses []DnsEntry

	for len(dnses) == 0 {
		dnses = Lookup()
	}

	svr := dnses[0]

	//if svr == nil {
	//	println("no ledslave found on network")
	//	os.Exit(1)
	//}

	//addr := fmt.Sprintf("ws://%s:%d", svr.Addr.String(), svr.Port)
	u := url.URL{
		Scheme: "ws",
		Host:   fmt.Sprintf("%v:%d", svr.IP, svr.Port),
		Path:   "/ws",
	}

	client, err := slave.Connect(u)
	errors.ExitIfErr(err)
	defer client.Close()

	err = client.SetBrightness(128)
	errors.ExitIfErr(err)

	dimension, err := client.GetDimension()
	errors.ExitIfErr(err)

	fmt.Printf("screen dimension is %dx%d", dimension.Width, dimension.Height)

	scrobe(client)
	//scanLines(client, dimension)

	//WaitForCtrlC()
}

func scanLines(client *slave.Client, dimension *slave.DimensionResponse) {
	colors := []color.RGBA{
		{R: 255},
		{G: 255},
		{B: 255},
	}

	horizontal := true
	it := int16(0)
	col := 0

	for {
		err := client.Clear(colors[col])
		errors.PrintIfErr(err)

		count := dimension.Width
		if !horizontal {
			count = dimension.Height
		}

		if horizontal {
			err = client.DrawLine(it, 0, it, int16(dimension.Height), White)
			errors.PrintIfErr(err)
		} else {
			err = client.DrawLine(0, it, int16(dimension.Width), it, White)
			errors.PrintIfErr(err)
		}

		it++

		if it >= int16(count) {
			horizontal = !horizontal
			it = 0
			col = (col + 1) % len(colors)
		}

		err = client.UpdateScreen()
		errors.PrintIfErr(err)
	}
}

func scrobe(client *slave.Client) {
	colors := []color.RGBA{Black, White}

	idx := 0
	for {
		idx = (idx + 1) % len(colors)

		err := client.Clear(colors[idx])
		errors.PrintIfErr(err)

		err = client.UpdateScreen()
		errors.PrintIfErr(err)

		time.Sleep(time.Millisecond * 17)
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

type DnsEntry struct {
	Id       string
	IP       net.IP
	Port     int
}

func Lookup() []DnsEntry {

	println("doing a mdns lookup for aurora devices")

	// Make a channel for results and start listening
	entriesCh := make(chan *mdns.ServiceEntry, 4)

	var dnss []DnsEntry

	wait := sync.WaitGroup{}
	go func() {
		wait.Add(1)
		for entry := range entriesCh {
			dnss = append(dnss, DnsEntry{
				IP:       entry.AddrV4,
				Port:     entry.Port,
				Id:       field(entry.InfoFields, "id"),
			})
		}
		wait.Done()
	}()

	// Start the lookup
	err := mdns.Lookup("_leds._tcp", entriesCh)
	errors.PrintIfErr(err)
	close(entriesCh)
	wait.Wait()

	fmt.Printf("found %d devices\n", len(dnss))

	return dnss
}


func field(fields []string, name string) string {
	for _, field := range fields {
		split := strings.SplitN(field, "=", 2)
		if len(split) > 1 && split[0] == name {
			return split[1]
		}
	}
	return ""
}

func search_dns() *zeroconf.ServiceEntry {
	// Discover all services on the network (e.g. _workstation._tcp)
	resolver, err := zeroconf.NewResolver(nil)
	if err != nil {
		log.Fatalln("Failed to initialize resolver:", err.Error())
	}

	var serv *zeroconf.ServiceEntry
	entries := make(chan *zeroconf.ServiceEntry)
	go func(results <-chan *zeroconf.ServiceEntry) {
		for entry := range results {
			log.Println(entry)
			serv = entry
		}
		log.Println("No more entries.")
	}(entries)

	//ctx, cancel := context.WithTimeout(context.Background(), time.Second * 10)
	//defer cancel()
	err = resolver.Browse(context.Background(), "_leds._tcp", "local", entries)
	if err != nil {
		log.Fatalln("Failed to browse:", err.Error())
	}

	//<-ctx.Done()
	// Wait some additional time to see debug messages on go routine shutdown.

	for serv == nil {
		time.Sleep(time.Millisecond * 100)
	}

	return serv
}
