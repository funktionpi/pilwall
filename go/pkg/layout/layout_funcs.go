package layout

//go:generate go-enum -f=$GOFILE --noprefix --names

// Layout x ENUM(
// RowMajorLayout,
// RowMajor90Layout,
// RowMajor180Layout,
// RowMajor270Layout,
// ColumnMajorLayout,
// ColumnMajor90Layout,
// ColumnMajor180Layout,
// ColumnMajor270Layout,
// RowMajorAlternatingLayout,
// RowMajorAlternating90Layout,
// RowMajorAlternating180Layout,
// RowMajorAlternating270Layout,
// ColumnMajorAlternatingLayout,
// ColumnMajorAlternating90Layout,
// ColumnMajorAlternating180Layout,
// ColumnMajorAlternating270Layout,
// )
type Layout int

func MustParseLayout(layout string) Layout {
	l, err := ParseLayout(layout)
	if err != nil {
		panic(err.Error())
	}
	return l
}

func (l Layout) Map(width, height uint16, x, y uint16) uint16 {
	switch l {
	case RowMajorLayout:
		return x + y*width
	case RowMajor90Layout:
		return (width-1-x)*height + y
	case RowMajor180Layout:
		return (width - 1 - x) + (height-1-y)*width
	case RowMajor270Layout:
		return x*height + (height - 1 - y)
	case ColumnMajorLayout:
		return x*height + y
	case ColumnMajor90Layout:
		return (width - 1 - x) + y*width
	case ColumnMajor180Layout:
		return (width-1-x)*height + (height - 1 - y)
	case ColumnMajor270Layout:
		return x + (height-1-y)*width
	case RowMajorAlternatingLayout:
		index := y * width
		if y&0x0001 != 0 {
			index += (width - 1) - x
		} else {
			index += x
		}
		return index
	case RowMajorAlternating90Layout:
		mx := (width - 1) - x
		index := mx * height
		if mx&0x0001 != 0 {
			index += (height - 1) - y
		} else {
			index += y
		}
		return index
	case RowMajorAlternating180Layout:
		my := (height - 1) - y
		index := my * width
		if my&0x0001 != 0 {
			index += x
		} else {
			index += (width - 1) - x
		}
		return index
	case RowMajorAlternating270Layout:
		index := x * height
		if x&0x0001 != 0 {
			index += y
		} else {
			index += (height - 1) - y
		}
		return index

	case ColumnMajorAlternatingLayout:
		index := x * height
		if x&0x0001 != 0 {
			index += (height - 1) - y
		} else {
			index += y
		}
		return index

	case ColumnMajorAlternating90Layout:
		index := y * width
		if y&0x0001 != 0 {
			index += x
		} else {
			index += (width - 1) - x
		}
		return index

	case ColumnMajorAlternating180Layout:
		mx := ((width - 1) - x)
		index := mx * height
		if mx&0x0001 != 0 {
			index += y
		} else {
			index += (height - 1) - y
		}
		return index

	case ColumnMajorAlternating270Layout:
		my := (height - 1) - y
		index := my * width
		if my&0x0001 != 0 {
			index += (width - 1) - x
		} else {
			index += x
		}
		return index

	default:
		panic("unknown pixel layout")
	}
}
