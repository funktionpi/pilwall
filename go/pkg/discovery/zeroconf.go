package discovery

import (
	"context"
	"fmt"
	"image"
	"net"
	"strconv"
	"strings"
	"time"

	"github.com/draeron/gopkg/errors"
	"github.com/grandcat/zeroconf"
)

type DnsEntry struct {
	Hostname string
	Ip       net.IP
	Port     int

	MatrixDimension image.Point
	TileDimension   image.Point
	TileCount       image.Point

	PanelLayout string
	TileLayout  string
}

const (
	MatrixWidth  = "matrix_width"
	MatrixHeight = "matrix_height"
	PanelWidth   = "panel_width"
	PanelHeight  = "panel_height"
	TilesX       = "tiles_horizontal"
	TilesY       = "tiles_vertical"
	PanelLayout  = "panel_layout"
	TileLayout   = "tile_layout"
)

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
				Hostname:        entry.HostName,
				Ip:              entry.AddrIPv4[0],
				Port:            entry.Port,
				MatrixDimension: fieldPt(entry, MatrixWidth, MatrixHeight),
				TileDimension:   fieldPt(entry, PanelWidth, PanelHeight),
				TileCount:       fieldPt(entry, TilesX, TilesY),
				PanelLayout:     field(entry, PanelLayout),
				TileLayout:      field(entry, TileLayout),
			})
			fmt.Printf("found '%s' @ '%s:%d'\n", entry.HostName, entry.AddrIPv4[0], entry.Port)
		}
	}(entries)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*3)
	defer cancel()
	err = resolver.Browse(ctx, "piproto._udp", "", entries)
	errors.ExitIfErr(err)

	<-ctx.Done()

	return dnss
}

func fieldPt(entry *zeroconf.ServiceEntry, namex, namey string) image.Point {
	dx, _ := strconv.Atoi(field(entry, namex))
	dy, _ := strconv.Atoi(field(entry, namey))
	return image.Pt(dx, dy)
}

func fieldInt(entry *zeroconf.ServiceEntry, name string) int {
	val, _ := strconv.Atoi(field(entry, name))
	return val
}

func field(entry *zeroconf.ServiceEntry, name string) string {
	for _, field := range entry.Text {
		split := strings.SplitN(field, "=", 2)
		if len(split) > 1 && split[0] == name {
			return split[1]
		}
	}
	return ""
}

/*

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

	err = resolver.Browse("pileds._udp", "local.", results)
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

		if (ifi.Flags&net.FlagMulticast) > 0 && (ifi.Name == "Ethernet" || ifi.Name == "Wi-Fi") {
			interfaces = append(interfaces, ifi)
		}
	}

	return interfaces
}

*/
