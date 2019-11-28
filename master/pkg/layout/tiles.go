package layout

type LayoutFunc func(width, height uint16, x, y uint16) uint16

var RowMajorLayout = func(width, height uint16, x, y uint16) uint16 {
	return x + y*width
}

var RowMajor90Layout = func(width, height uint16, x, y uint16) uint16 {
	return (width-1-x)*height + y
}

var RowMajor180Layout = func(width, height uint16, x, y uint16) uint16 {
	return (width - 1 - x) + (height-1-y)*width
}

var RowMajor270Layout = func(width, height uint16, x, y uint16) uint16 {
	return x*height + (height - 1 - y)
}

var ColumnMajorLayout = func(width, height uint16, x, y uint16) uint16 {
	return x*height + y
}

var ColumnMajor90Layout = func(width, height uint16, x, y uint16) uint16 {
	return (width - 1 - x) + y*width
}

var ColumnMajor180Layout = func(width, height uint16, x, y uint16) uint16 {
	return (width-1-x)*height + (height - 1 - y)
}

var ColumnMajor270Layout = func(width, height uint16, x, y uint16) uint16 {
	return x + (height-1-y)*width
}

var RowMajorAlternatingLayout = func(width, height uint16, x, y uint16) uint16 {
	index := y * width
	if y&0x0001 != 0 {
		index += (width - 1) - x
	} else {
		index += x
	}
	return index
}

var RowMajorAlternating90Layout = func(width, height uint16, x, y uint16) uint16 {
	mx := (width - 1) - x
	index := mx * height
	if mx&0x0001 != 0 {
		index += (height - 1) - y
	} else {
		index += y
	}
	return index
}

var RowMajorAlternating180Layout = func(width, height uint16, x, y uint16) uint16 {
	my := (height - 1) - y
	index := my * width
	if my&0x0001 != 0 {
		index += x
	} else {
		index += (width - 1) - x
	}
	return index
}

var RowMajorAlternating270Layout = func(width, height uint16, x, y uint16) uint16 {
	index := x * height
	if x&0x0001 != 0 {
		index += y
	} else {
		index += (height - 1) - y
	}
	return index
}

var ColumnMajorAlternatingLayout = func(width, height uint16, x, y uint16) uint16 {
	index := x * height
	if x&0x0001 != 0 {
		index += (height - 1) - y
	} else {
		index += y
	}
	return index
}

var ColumnMajorAlternating90Layout = func(width, height uint16, x, y uint16) uint16 {
	index := y * width
	if y&0x0001 != 0 {
		index += x
	} else {
		index += (width - 1) - x
	}
	return index
}

var ColumnMajorAlternating180Layout = func(width, height uint16, x, y uint16) uint16 {
	mx := ((width - 1) - x)
	index := mx * height
	if mx&0x0001 != 0 {
		index += y
	} else {
		index += (height - 1) - y
	}
	return index
}

var ColumnMajorAlternating270Layout = func(width, height uint16, x, y uint16) uint16 {
	my := (height - 1) - y
	index := my * width
	if my&0x0001 != 0 {
		index += (width - 1) - x
	} else {
		index += x
	}
	return index
}
