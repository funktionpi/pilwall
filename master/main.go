package main

import (
	"context"
	"fmt"
	"github.com/grandcat/zeroconf"
	"github.com/oleksandr/bonjour"
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
		dnses = LookupZeroconf()
	}

	svr := dnses[0]

	//if svr == nil {
	//	println("no ledslave found on network")
	//	os.Exit(1)
	//}

	//u := url.URL{
	//	Scheme: "ws",
	//	Host:   fmt.Sprintf("%v:%d", svr.IP, svr.Port),
	//	Path:   "/ws",
	//}
	u := url.URL{Host: fmt.Sprintf("%v:%d", svr.IP, 1234)}

	client, err := slave.Connect(u)
	errors.ExitIfErr(err)
	defer client.Close()

	err = client.SetBrightness(4)
	errors.ExitIfErr(err)

	dimension, err := client.GetDimension()
	errors.ExitIfErr(err)

	fmt.Printf("screen dimension is %dx%d\n", dimension.Width, dimension.Height)

	//scrobe(client)
	scanLines(client, dimension)

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

		//time.Sleep(time.Second / 10)
	}
}

func scrobe(client *slave.Client) {
	colors := []color.RGBA{Black, White, Black, Red, Black, Blue, Black, Green}

	idx := 0
	for {
		idx = (idx + 1) % len(colors)

		err := client.Clear(colors[idx])
		errors.PrintIfErr(err)

		err = client.UpdateScreen()
		errors.PrintIfErr(err)

		//time.Sleep(time.Millisecond * 17)
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

func LookupMDNS() []DnsEntry {

	println("doing a mdns lookup for led devices")

	resolver, err := bonjour.NewResolver(nil)
	if err != nil {
		log.Println("Failed to initialize resolver:", err.Error())
		os.Exit(1)
	}

	results := make(chan *bonjour.ServiceEntry, 8)

	go func(results chan *bonjour.ServiceEntry, exitCh chan<- bool) {
		for e := range results {
			log.Printf("%s", e.Instance)
			exitCh <- true
			time.Sleep(1e9)
			os.Exit(0)
		}
	}(results, resolver.Exit)

	err = resolver.Browse("_leds._tcp", "local.", results)
	if err != nil {
		log.Println("Failed to browse:", err.Error())
	}

	dnss := []DnsEntry{}
	for e := range results {
		dnss = append(dnss, DnsEntry{
			Id:   e.HostName,
			IP:   e.AddrIPv4,
			Port: e.Port,
		})
	}

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

func LookupZeroconf() []DnsEntry {

	// Discover all services on the network (e.g. _workstation._tcp)
	resolver, err := zeroconf.NewResolver(zeroconf.SelectIPTraffic(zeroconf.IPv4))
	if err != nil {
		errors.ExitIfErr(err)
		//log.Fatalln("Failed to initialize resolver:", err.Error())
	}

	dnss := []DnsEntry{}

	entries := make(chan *zeroconf.ServiceEntry)
	go func(results <-chan *zeroconf.ServiceEntry) {
		for entry := range results {
			dnss = append(dnss, DnsEntry{
				Id:   entry.HostName,
				IP:   entry.AddrIPv4[0],
				Port: entry.Port,
			})
			fmt.Printf("found '%s' @ '%s:%d'\n", entry.HostName, entry.AddrIPv4[0], entry.Port)
		}
	}(entries)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel()
	err = resolver.Browse(ctx, "_leds._tcp", "", entries)
	errors.ExitIfErr(err)

	<-ctx.Done()

	return dnss
}

func listInterfaces() []net.Interface {
	var interfaces []net.Interface
	ifaces, err := net.Interfaces()
	if err != nil {
		return nil
	}
	for _, ifi := range ifaces {
		if (ifi.Flags & net.FlagUp) == 0 {
			continue
		}

		if (ifi.Flags & net.FlagMulticast) > 0 && (ifi.Name == "Ethernet" || ifi.Name == "Wi-Fi") {
			interfaces = append(interfaces, ifi)
		}
	}

	return interfaces
}
