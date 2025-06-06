const char* const large_nested_abi = R"=====(
{
  "____comment": "",
  "version": "sysio::abi/1.0",
  "types": [],
  "structs": [
	{
      "name": "s0",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "int64"
        }
      ]
    }
        
{
      "name": "s1",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s0"
        }
	 {
          "name": "f1",
          "type": "s0"
        }
	 {
          "name": "f1",
          "type": "s0"
        }
      ]
    }

{
      "name": "s2",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s1"
        }
	 {
          "name": "f1",
          "type": "s1"
        }
	 {
          "name": "f1",
          "type": "s1"
        }
      ]
    }

{
      "name": "s3",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s2"
        }
	 {
          "name": "f1",
          "type": "s2"
        }
	 {
          "name": "f1",
          "type": "s2"
        }
      ]
    }

{
      "name": "s4",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s3"
        }
	 {
          "name": "f1",
          "type": "s3"
        }
	 {
          "name": "f1",
          "type": "s3"
        }
      ]
    }

{
      "name": "s5",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s4"
        }
	 {
          "name": "f1",
          "type": "s4"
        }
	 {
          "name": "f1",
          "type": "s4"
        }
      ]
    }

{
      "name": "s6",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s5"
        }
	 {
          "name": "f1",
          "type": "s5"
        }
	 {
          "name": "f1",
          "type": "s5"
        }
      ]
    }

{
      "name": "s7",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s6"
        }
	 {
          "name": "f1",
          "type": "s6"
        }
	 {
          "name": "f1",
          "type": "s6"
        }
      ]
    }

{
      "name": "s8",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s7"
        }
	 {
          "name": "f1",
          "type": "s7"
        }
	 {
          "name": "f1",
          "type": "s7"
        }
      ]
    }

{
      "name": "s9",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s8"
        }
	 {
          "name": "f1",
          "type": "s8"
        }
	 {
          "name": "f1",
          "type": "s8"
        }
      ]
    }

{
      "name": "s10",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s9"
        }
	 {
          "name": "f1",
          "type": "s9"
        }
	 {
          "name": "f1",
          "type": "s9"
        }
      ]
    }

{
      "name": "s11",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s10"
        }
	 {
          "name": "f1",
          "type": "s10"
        }
	 {
          "name": "f1",
          "type": "s10"
        }
      ]
    }

{
      "name": "s12",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s11"
        }
	 {
          "name": "f1",
          "type": "s11"
        }
	 {
          "name": "f1",
          "type": "s11"
        }
      ]
    }

{
      "name": "s13",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s12"
        }
	 {
          "name": "f1",
          "type": "s12"
        }
	 {
          "name": "f1",
          "type": "s12"
        }
      ]
    }

{
      "name": "s14",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s13"
        }
	 {
          "name": "f1",
          "type": "s13"
        }
	 {
          "name": "f1",
          "type": "s13"
        }
      ]
    }

{
      "name": "s15",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s14"
        }
	 {
          "name": "f1",
          "type": "s14"
        }
	 {
          "name": "f1",
          "type": "s14"
        }
      ]
    }

{
      "name": "s16",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s15"
        }
	 {
          "name": "f1",
          "type": "s15"
        }
	 {
          "name": "f1",
          "type": "s15"
        }
      ]
    }

{
      "name": "s17",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s16"
        }
	 {
          "name": "f1",
          "type": "s16"
        }
	 {
          "name": "f1",
          "type": "s16"
        }
      ]
    }

{
      "name": "s18",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s17"
        }
	 {
          "name": "f1",
          "type": "s17"
        }
	 {
          "name": "f1",
          "type": "s17"
        }
      ]
    }

{
      "name": "s19",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s18"
        }
	 {
          "name": "f1",
          "type": "s18"
        }
	 {
          "name": "f1",
          "type": "s18"
        }
      ]
    }

{
      "name": "s20",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s19"
        }
	 {
          "name": "f1",
          "type": "s19"
        }
	 {
          "name": "f1",
          "type": "s19"
        }
      ]
    }

{
      "name": "s21",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s20"
        }
	 {
          "name": "f1",
          "type": "s20"
        }
	 {
          "name": "f1",
          "type": "s20"
        }
      ]
    }

{
      "name": "s22",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s21"
        }
	 {
          "name": "f1",
          "type": "s21"
        }
	 {
          "name": "f1",
          "type": "s21"
        }
      ]
    }

{
      "name": "s23",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s22"
        }
	 {
          "name": "f1",
          "type": "s22"
        }
	 {
          "name": "f1",
          "type": "s22"
        }
      ]
    }

{
      "name": "s24",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s23"
        }
	 {
          "name": "f1",
          "type": "s23"
        }
	 {
          "name": "f1",
          "type": "s23"
        }
      ]
    }

{
      "name": "s25",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s24"
        }
	 {
          "name": "f1",
          "type": "s24"
        }
	 {
          "name": "f1",
          "type": "s24"
        }
      ]
    }

{
      "name": "s26",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s25"
        }
	 {
          "name": "f1",
          "type": "s25"
        }
	 {
          "name": "f1",
          "type": "s25"
        }
      ]
    }

{
      "name": "s27",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s26"
        }
	 {
          "name": "f1",
          "type": "s26"
        }
	 {
          "name": "f1",
          "type": "s26"
        }
      ]
    }

{
      "name": "s28",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s27"
        }
	 {
          "name": "f1",
          "type": "s27"
        }
	 {
          "name": "f1",
          "type": "s27"
        }
      ]
    }

{
      "name": "s29",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s28"
        }
	 {
          "name": "f1",
          "type": "s28"
        }
	 {
          "name": "f1",
          "type": "s28"
        }
      ]
    }

{
      "name": "s30",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s29"
        }
	 {
          "name": "f1",
          "type": "s29"
        }
	 {
          "name": "f1",
          "type": "s29"
        }
      ]
    }

{
      "name": "s31",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s30"
        }
	 {
          "name": "f1",
          "type": "s30"
        }
	 {
          "name": "f1",
          "type": "s30"
        }
      ]
    }

{
      "name": "s32",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s31"
        }
	 {
          "name": "f1",
          "type": "s31"
        }
	 {
          "name": "f1",
          "type": "s31"
        }
      ]
    }

{
      "name": "s33",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s32"
        }
	 {
          "name": "f1",
          "type": "s32"
        }
	 {
          "name": "f1",
          "type": "s32"
        }
      ]
    }

{
      "name": "s34",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s33"
        }
	 {
          "name": "f1",
          "type": "s33"
        }
	 {
          "name": "f1",
          "type": "s33"
        }
      ]
    }

{
      "name": "s35",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s34"
        }
	 {
          "name": "f1",
          "type": "s34"
        }
	 {
          "name": "f1",
          "type": "s34"
        }
      ]
    }

{
      "name": "s36",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s35"
        }
	 {
          "name": "f1",
          "type": "s35"
        }
	 {
          "name": "f1",
          "type": "s35"
        }
      ]
    }

{
      "name": "s37",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s36"
        }
	 {
          "name": "f1",
          "type": "s36"
        }
	 {
          "name": "f1",
          "type": "s36"
        }
      ]
    }

{
      "name": "s38",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s37"
        }
	 {
          "name": "f1",
          "type": "s37"
        }
	 {
          "name": "f1",
          "type": "s37"
        }
      ]
    }

{
      "name": "s39",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s38"
        }
	 {
          "name": "f1",
          "type": "s38"
        }
	 {
          "name": "f1",
          "type": "s38"
        }
      ]
    }

{
      "name": "s40",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s39"
        }
	 {
          "name": "f1",
          "type": "s39"
        }
	 {
          "name": "f1",
          "type": "s39"
        }
      ]
    }

{
      "name": "s41",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s40"
        }
	 {
          "name": "f1",
          "type": "s40"
        }
	 {
          "name": "f1",
          "type": "s40"
        }
      ]
    }

{
      "name": "s42",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s41"
        }
	 {
          "name": "f1",
          "type": "s41"
        }
	 {
          "name": "f1",
          "type": "s41"
        }
      ]
    }

{
      "name": "s43",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s42"
        }
	 {
          "name": "f1",
          "type": "s42"
        }
	 {
          "name": "f1",
          "type": "s42"
        }
      ]
    }

{
      "name": "s44",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s43"
        }
	 {
          "name": "f1",
          "type": "s43"
        }
	 {
          "name": "f1",
          "type": "s43"
        }
      ]
    }

{
      "name": "s45",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s44"
        }
	 {
          "name": "f1",
          "type": "s44"
        }
	 {
          "name": "f1",
          "type": "s44"
        }
      ]
    }

{
      "name": "s46",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s45"
        }
	 {
          "name": "f1",
          "type": "s45"
        }
	 {
          "name": "f1",
          "type": "s45"
        }
      ]
    }

{
      "name": "s47",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s46"
        }
	 {
          "name": "f1",
          "type": "s46"
        }
	 {
          "name": "f1",
          "type": "s46"
        }
      ]
    }

{
      "name": "s48",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s47"
        }
	 {
          "name": "f1",
          "type": "s47"
        }
	 {
          "name": "f1",
          "type": "s47"
        }
      ]
    }

{
      "name": "s49",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s48"
        }
	 {
          "name": "f1",
          "type": "s48"
        }
	 {
          "name": "f1",
          "type": "s48"
        }
      ]
    }

{
      "name": "s50",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s49"
        }
	 {
          "name": "f1",
          "type": "s49"
        }
	 {
          "name": "f1",
          "type": "s49"
        }
      ]
    }

{
      "name": "s51",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s50"
        }
	 {
          "name": "f1",
          "type": "s50"
        }
	 {
          "name": "f1",
          "type": "s50"
        }
      ]
    }

{
      "name": "s52",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s51"
        }
	 {
          "name": "f1",
          "type": "s51"
        }
	 {
          "name": "f1",
          "type": "s51"
        }
      ]
    }

{
      "name": "s53",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s52"
        }
	 {
          "name": "f1",
          "type": "s52"
        }
	 {
          "name": "f1",
          "type": "s52"
        }
      ]
    }

{
      "name": "s54",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s53"
        }
	 {
          "name": "f1",
          "type": "s53"
        }
	 {
          "name": "f1",
          "type": "s53"
        }
      ]
    }

{
      "name": "s55",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s54"
        }
	 {
          "name": "f1",
          "type": "s54"
        }
	 {
          "name": "f1",
          "type": "s54"
        }
      ]
    }

{
      "name": "s56",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s55"
        }
	 {
          "name": "f1",
          "type": "s55"
        }
	 {
          "name": "f1",
          "type": "s55"
        }
      ]
    }

{
      "name": "s57",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s56"
        }
	 {
          "name": "f1",
          "type": "s56"
        }
	 {
          "name": "f1",
          "type": "s56"
        }
      ]
    }

{
      "name": "s58",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s57"
        }
	 {
          "name": "f1",
          "type": "s57"
        }
	 {
          "name": "f1",
          "type": "s57"
        }
      ]
    }

{
      "name": "s59",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s58"
        }
	 {
          "name": "f1",
          "type": "s58"
        }
	 {
          "name": "f1",
          "type": "s58"
        }
      ]
    }

{
      "name": "s60",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s59"
        }
	 {
          "name": "f1",
          "type": "s59"
        }
	 {
          "name": "f1",
          "type": "s59"
        }
      ]
    }

{
      "name": "s61",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s60"
        }
	 {
          "name": "f1",
          "type": "s60"
        }
	 {
          "name": "f1",
          "type": "s60"
        }
      ]
    }

{
      "name": "s62",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s61"
        }
	 {
          "name": "f1",
          "type": "s61"
        }
	 {
          "name": "f1",
          "type": "s61"
        }
      ]
    }

{
      "name": "s63",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s62"
        }
	 {
          "name": "f1",
          "type": "s62"
        }
	 {
          "name": "f1",
          "type": "s62"
        }
      ]
    }

{
      "name": "s64",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s63"
        }
	 {
          "name": "f1",
          "type": "s63"
        }
	 {
          "name": "f1",
          "type": "s63"
        }
      ]
    }

{
      "name": "s65",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s64"
        }
	 {
          "name": "f1",
          "type": "s64"
        }
	 {
          "name": "f1",
          "type": "s64"
        }
      ]
    }

{
      "name": "s66",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s65"
        }
	 {
          "name": "f1",
          "type": "s65"
        }
	 {
          "name": "f1",
          "type": "s65"
        }
      ]
    }

{
      "name": "s67",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s66"
        }
	 {
          "name": "f1",
          "type": "s66"
        }
	 {
          "name": "f1",
          "type": "s66"
        }
      ]
    }

{
      "name": "s68",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s67"
        }
	 {
          "name": "f1",
          "type": "s67"
        }
	 {
          "name": "f1",
          "type": "s67"
        }
      ]
    }

{
      "name": "s69",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s68"
        }
	 {
          "name": "f1",
          "type": "s68"
        }
	 {
          "name": "f1",
          "type": "s68"
        }
      ]
    }

{
      "name": "s70",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s69"
        }
	 {
          "name": "f1",
          "type": "s69"
        }
	 {
          "name": "f1",
          "type": "s69"
        }
      ]
    }

{
      "name": "s71",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s70"
        }
	 {
          "name": "f1",
          "type": "s70"
        }
	 {
          "name": "f1",
          "type": "s70"
        }
      ]
    }

{
      "name": "s72",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s71"
        }
	 {
          "name": "f1",
          "type": "s71"
        }
	 {
          "name": "f1",
          "type": "s71"
        }
      ]
    }

{
      "name": "s73",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s72"
        }
	 {
          "name": "f1",
          "type": "s72"
        }
	 {
          "name": "f1",
          "type": "s72"
        }
      ]
    }

{
      "name": "s74",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s73"
        }
	 {
          "name": "f1",
          "type": "s73"
        }
	 {
          "name": "f1",
          "type": "s73"
        }
      ]
    }

{
      "name": "s75",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s74"
        }
	 {
          "name": "f1",
          "type": "s74"
        }
	 {
          "name": "f1",
          "type": "s74"
        }
      ]
    }

{
      "name": "s76",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s75"
        }
	 {
          "name": "f1",
          "type": "s75"
        }
	 {
          "name": "f1",
          "type": "s75"
        }
      ]
    }

{
      "name": "s77",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s76"
        }
	 {
          "name": "f1",
          "type": "s76"
        }
	 {
          "name": "f1",
          "type": "s76"
        }
      ]
    }

{
      "name": "s78",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s77"
        }
	 {
          "name": "f1",
          "type": "s77"
        }
	 {
          "name": "f1",
          "type": "s77"
        }
      ]
    }

{
      "name": "s79",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s78"
        }
	 {
          "name": "f1",
          "type": "s78"
        }
	 {
          "name": "f1",
          "type": "s78"
        }
      ]
    }

{
      "name": "s80",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s79"
        }
	 {
          "name": "f1",
          "type": "s79"
        }
	 {
          "name": "f1",
          "type": "s79"
        }
      ]
    }

{
      "name": "s81",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s80"
        }
	 {
          "name": "f1",
          "type": "s80"
        }
	 {
          "name": "f1",
          "type": "s80"
        }
      ]
    }

{
      "name": "s82",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s81"
        }
	 {
          "name": "f1",
          "type": "s81"
        }
	 {
          "name": "f1",
          "type": "s81"
        }
      ]
    }

{
      "name": "s83",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s82"
        }
	 {
          "name": "f1",
          "type": "s82"
        }
	 {
          "name": "f1",
          "type": "s82"
        }
      ]
    }

{
      "name": "s84",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s83"
        }
	 {
          "name": "f1",
          "type": "s83"
        }
	 {
          "name": "f1",
          "type": "s83"
        }
      ]
    }

{
      "name": "s85",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s84"
        }
	 {
          "name": "f1",
          "type": "s84"
        }
	 {
          "name": "f1",
          "type": "s84"
        }
      ]
    }

{
      "name": "s86",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s85"
        }
	 {
          "name": "f1",
          "type": "s85"
        }
	 {
          "name": "f1",
          "type": "s85"
        }
      ]
    }

{
      "name": "s87",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s86"
        }
	 {
          "name": "f1",
          "type": "s86"
        }
	 {
          "name": "f1",
          "type": "s86"
        }
      ]
    }

{
      "name": "s88",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s87"
        }
	 {
          "name": "f1",
          "type": "s87"
        }
	 {
          "name": "f1",
          "type": "s87"
        }
      ]
    }

{
      "name": "s89",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s88"
        }
	 {
          "name": "f1",
          "type": "s88"
        }
	 {
          "name": "f1",
          "type": "s88"
        }
      ]
    }

{
      "name": "s90",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s89"
        }
	 {
          "name": "f1",
          "type": "s89"
        }
	 {
          "name": "f1",
          "type": "s89"
        }
      ]
    }

{
      "name": "s91",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s90"
        }
	 {
          "name": "f1",
          "type": "s90"
        }
	 {
          "name": "f1",
          "type": "s90"
        }
      ]
    }

{
      "name": "s92",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s91"
        }
	 {
          "name": "f1",
          "type": "s91"
        }
	 {
          "name": "f1",
          "type": "s91"
        }
      ]
    }

{
      "name": "s93",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s92"
        }
	 {
          "name": "f1",
          "type": "s92"
        }
	 {
          "name": "f1",
          "type": "s92"
        }
      ]
    }

{
      "name": "s94",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s93"
        }
	 {
          "name": "f1",
          "type": "s93"
        }
	 {
          "name": "f1",
          "type": "s93"
        }
      ]
    }

{
      "name": "s95",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s94"
        }
	 {
          "name": "f1",
          "type": "s94"
        }
	 {
          "name": "f1",
          "type": "s94"
        }
      ]
    }

{
      "name": "s96",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s95"
        }
	 {
          "name": "f1",
          "type": "s95"
        }
	 {
          "name": "f1",
          "type": "s95"
        }
      ]
    }

{
      "name": "s97",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s96"
        }
	 {
          "name": "f1",
          "type": "s96"
        }
	 {
          "name": "f1",
          "type": "s96"
        }
      ]
    }

{
      "name": "s98",
      "base": "",
      "fields": [
        {
          "name": "f1",
          "type": "s97"
        }
	 {
          "name": "f1",
          "type": "s97"
        }
	 {
          "name": "f1",
          "type": "s97"
        }
      ]
    }

  ],
  "actions": [{
      "name": "hi",
      "type": "s98",
      "ricardian_contract": ""
    }
  ],
  "tables": [],
  "ricardian_clauses": [],
  "error_messages": [],
  "abi_extensions": []
}
)=====";
