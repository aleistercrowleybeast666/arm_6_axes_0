var edaEsbuildExportName = (() => {
  var __defProp = Object.defineProperty;
  var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
  var __getOwnPropNames = Object.getOwnPropertyNames;
  var __hasOwnProp = Object.prototype.hasOwnProperty;
  var __export = (target, all) => {
    for (var name in all)
      __defProp(target, name, { get: all[name], enumerable: true });
  };
  var __copyProps = (to, from, except, desc) => {
    if (from && typeof from === "object" || typeof from === "function") {
      for (let key of __getOwnPropNames(from))
        if (!__hasOwnProp.call(to, key) && key !== except)
          __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
    }
    return to;
  };
  var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

  // src/index.js
  var index_exports = {};
  __export(index_exports, {
    generateRevA: () => generateRevA
  });

  // ../../design/controller_revA.json
  var controller_revA_default = {
    name: "arm_6_axes_0_Prototype0_RevA",
    revision: "A",
    sheets: [
      "01_POWER",
      "02_MCU",
      "03_CAN",
      "04_STORAGE",
      "05_UI_IO",
      "06_DEBUG"
    ],
    components: [
      {
        ref: "J_POWER",
        part: "xt30",
        sheet: "01_POWER",
        pins: {
          "1": "24V_IN",
          "2": "GND"
        },
        x: 80,
        y: 120,
        notes: "PIN1 +24V_LOGIC; PIN2 GND. Logic branch only; verify moulded +/\u2212 against library pads.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "F1",
        part: "fuse",
        sheet: "01_POWER",
        pins: {
          "1": "24V_IN",
          "2": "VIN_FUSED"
        },
        x: 230,
        y: 120,
        notes: "1A fast fuse; upstream battery branch fuse required; interrupt rating coordination pending.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "D_REVERSE",
        part: "reverse",
        sheet: "01_POWER",
        pins: {
          "1": "VIN_PROTECTED",
          "2": "VIN_FUSED"
        },
        x: 390,
        y: 120,
        notes: "100V/3A series Schottky; cathode downstream.",
        pinNames: {
          "1": "K",
          "2": "A"
        },
        dnp: false
      },
      {
        ref: "D_INPUT",
        part: "input_tvs",
        sheet: "01_POWER",
        pins: {
          "1": "VIN_PROTECTED",
          "2": "GND"
        },
        x: 570,
        y: 120,
        notes: "SMBJ33A, VRWM33V, VC53.3V at rated pulse.",
        pinNames: {
          "1": "K",
          "2": "A"
        },
        dnp: false
      },
      {
        ref: "C_IN_BULK",
        part: "47uHV",
        sheet: "01_POWER",
        pins: {
          "1": "VIN_PROTECTED",
          "2": "GND"
        },
        x: 740,
        y: 120,
        notes: "47uF/63V polymer. Pin1 positive.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_IN",
        part: "4u7HV",
        sheet: "01_POWER",
        pins: {
          "1": "VIN_PROTECTED",
          "2": "GND"
        },
        x: 910,
        y: 120,
        notes: "4.7uF/100V X7R; immediately across VIN/GND.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_IN_HF",
        part: "100n100",
        sheet: "01_POWER",
        pins: {
          "1": "VIN_PROTECTED",
          "2": "GND"
        },
        x: 1080,
        y: 120,
        notes: "100nF/100V; minimum hot-loop area.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "U_BUCK",
        part: "buck",
        sheet: "01_POWER",
        pins: {
          "1": "GND",
          "2": "VIN_PROTECTED",
          "3": "VIN_PROTECTED",
          "4": "BUCK_RT",
          "5": "BUCK_FB",
          "6": "BUCK_PG",
          "7": "BUCK_BOOT",
          "8": "BUCK_SW",
          "9": "GND"
        },
        x: 300,
        y: 420,
        notes: "CRITICAL LAYOUT AREA. EN=VIN; exposed pad GND. No external VCC pin.",
        pinNames: {
          "1": "GND",
          "2": "EN",
          "3": "VIN",
          "4": "RT/SYNC",
          "5": "FB",
          "6": "PG",
          "7": "BOOT",
          "8": "SW",
          "9": "EP"
        },
        dnp: false
      },
      {
        ref: "R_RT",
        part: "64k9",
        sheet: "01_POWER",
        pins: {
          "1": "BUCK_RT",
          "2": "GND"
        },
        x: 70,
        y: 420,
        notes: "400kHz, TI Table 8-1.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_BOOT",
        part: "100n",
        sheet: "01_POWER",
        pins: {
          "1": "BUCK_BOOT",
          "2": "BUCK_SW"
        },
        x: 520,
        y: 360,
        notes: "100nF directly BOOT to SW, not GND.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "L1",
        part: "inductor",
        sheet: "01_POWER",
        pins: {
          "1": "BUCK_SW",
          "2": "5V"
        },
        x: 720,
        y: 420,
        notes: "15uH, 4.5A thermal/8A saturation at specified test conditions.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_OUT1",
        part: "22u",
        sheet: "01_POWER",
        pins: {
          "1": "5V",
          "2": "GND"
        },
        x: 900,
        y: 420,
        notes: "22uF/25V X7R, 1210; verify DC-bias effective capacitance.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_OUT2",
        part: "22u",
        sheet: "01_POWER",
        pins: {
          "1": "5V",
          "2": "GND"
        },
        x: 1060,
        y: 420,
        notes: "22uF/25V X7R, 1210; verify DC-bias effective capacitance.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_OUT3",
        part: "22u",
        sheet: "01_POWER",
        pins: {
          "1": "5V",
          "2": "GND"
        },
        x: 1220,
        y: 420,
        notes: "22uF/25V X7R, 1210; verify DC-bias effective capacitance.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_FB_TOP",
        part: "100k",
        sheet: "01_POWER",
        pins: {
          "1": "5V",
          "2": "BUCK_FB"
        },
        x: 540,
        y: 550,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_FB_BOTTOM",
        part: "24k9",
        sheet: "01_POWER",
        pins: {
          "1": "BUCK_FB",
          "2": "GND"
        },
        x: 740,
        y: 550,
        notes: "Vout=1V*(1+100k/24.9k)=5.016V.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_PG",
        part: "100k",
        sheet: "01_POWER",
        pins: {
          "1": "3V3",
          "2": "BUCK_PG"
        },
        x: 900,
        y: 550,
        notes: "Open drain PG pulled to 3V3; debug test point only.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "U_LDO",
        part: "ldo",
        sheet: "01_POWER",
        pins: {
          "1": "3V3",
          "2": "5V",
          "3": null,
          "4": "GND",
          "5": null,
          "6": null,
          "7": null,
          "8": "5V",
          "9": "GND"
        },
        x: 300,
        y: 800,
        notes: "AP7361C SO-8EP (SP), EN=IN. Thermal design limits continuous current.",
        pinNames: {
          "1": "OUT",
          "2": "EN",
          "3": "NC",
          "4": "GND",
          "5": "NC",
          "6": "NC",
          "7": "NC",
          "8": "IN",
          "9": "EP"
        },
        dnp: false
      },
      {
        ref: "C_LDO_IN",
        part: "10u",
        sheet: "01_POWER",
        pins: {
          "1": "5V",
          "2": "GND"
        },
        x: 70,
        y: 800,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_LDO_OUT",
        part: "10u",
        sheet: "01_POWER",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 550,
        y: 800,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_LED",
        part: "470R",
        sheet: "01_POWER",
        pins: {
          "1": "3V3",
          "2": "LED_A"
        },
        x: 780,
        y: 800,
        notes: "About 1mA at Vf=2.83V; verify LED batch at low current.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "LED_PWR",
        part: "led",
        sheet: "01_POWER",
        pins: {
          "1": "GND",
          "2": "LED_A"
        },
        x: 1e3,
        y: 800,
        notes: "BLUE 0805; silk PWR.",
        pinNames: {
          "1": "K",
          "2": "A"
        },
        dnp: false
      },
      {
        ref: "U_MCU",
        part: "mcu",
        sheet: "02_MCU",
        pins: {
          "1": "BTN_FORWARD",
          "2": "BTN_BACK",
          "3": "BTN_SOFTSTOP",
          "4": null,
          "5": null,
          "6": "3V3",
          "7": null,
          "8": null,
          "9": null,
          "10": "GND",
          "11": "3V3",
          "12": "HSE_IN",
          "13": "HSE_OUT",
          "14": "NRST",
          "15": null,
          "16": null,
          "17": null,
          "18": null,
          "19": "3V3",
          "20": "GND",
          "21": "VDDA",
          "22": "VDDA",
          "23": null,
          "24": null,
          "25": null,
          "26": null,
          "27": "GND",
          "28": "3V3",
          "29": null,
          "30": null,
          "31": "ENC_A",
          "32": "ENC_B",
          "33": null,
          "34": null,
          "35": null,
          "36": null,
          "37": "BOOT1",
          "38": null,
          "39": null,
          "40": null,
          "41": null,
          "42": null,
          "43": null,
          "44": null,
          "45": null,
          "46": null,
          "47": null,
          "48": null,
          "49": "VCAP1",
          "50": "3V3",
          "51": null,
          "52": null,
          "53": null,
          "54": null,
          "55": null,
          "56": null,
          "57": null,
          "58": null,
          "59": null,
          "60": null,
          "61": null,
          "62": null,
          "63": null,
          "64": "SD_DETECT",
          "65": "SD_D0",
          "66": "SD_D1",
          "67": null,
          "68": "UART_TX",
          "69": "UART_RX",
          "70": "USB_DM_RESERVED",
          "71": "USB_DP_RESERVED",
          "72": "SWDIO",
          "73": "VCAP2",
          "74": "GND",
          "75": "3V3",
          "76": "SWCLK",
          "77": null,
          "78": "SD_D2",
          "79": "SD_D3",
          "80": "SD_CLK_MCU",
          "81": null,
          "82": null,
          "83": "SD_CMD",
          "84": null,
          "85": null,
          "86": null,
          "87": null,
          "88": null,
          "89": null,
          "90": null,
          "91": null,
          "92": "OLED_SCL",
          "93": "OLED_SDA",
          "94": "BOOT0",
          "95": "CAN_RX",
          "96": "CAN_TX",
          "97": "HX_DOUT_MCU",
          "98": "HX_SCK_MCU",
          "99": "GND",
          "100": "3V3"
        },
        x: 420,
        y: 600,
        notes: "All unused GPIO explicitly no-connect; no pin reassignment.",
        pinNames: {
          "1": "PE2",
          "2": "PE3",
          "3": "PE4",
          "4": "PE5",
          "5": "PE6",
          "6": "VBAT",
          "7": "PC13",
          "8": "PC14",
          "9": "PC15",
          "10": "VSS",
          "11": "VDD",
          "12": "PH0",
          "13": "PH1",
          "14": "NRST",
          "15": "PC0",
          "16": "PC1",
          "17": "PC2",
          "18": "PC3",
          "19": "VDD",
          "20": "VSSA",
          "21": "VREF+",
          "22": "VDDA",
          "23": "PA0",
          "24": "PA1",
          "25": "PA2",
          "26": "PA3",
          "27": "VSS",
          "28": "VDD",
          "29": "PA4",
          "30": "PA5",
          "31": "PA6",
          "32": "PA7",
          "33": "PC4",
          "34": "PC5",
          "35": "PB0",
          "36": "PB1",
          "37": "PB2",
          "38": "PE7",
          "39": "PE8",
          "40": "PE9",
          "41": "PE10",
          "42": "PE11",
          "43": "PE12",
          "44": "PE13",
          "45": "PE14",
          "46": "PE15",
          "47": "PB10",
          "48": "PB11",
          "49": "VCAP_1",
          "50": "VDD",
          "51": "PB12",
          "52": "PB13",
          "53": "PB14",
          "54": "PB15",
          "55": "PD8",
          "56": "PD9",
          "57": "PD10",
          "58": "PD11",
          "59": "PD12",
          "60": "PD13",
          "61": "PD14",
          "62": "PD15",
          "63": "PC6",
          "64": "PC7",
          "65": "PC8",
          "66": "PC9",
          "67": "PA8",
          "68": "PA9",
          "69": "PA10",
          "70": "PA11",
          "71": "PA12",
          "72": "PA13",
          "73": "VCAP_2",
          "74": "VSS",
          "75": "VDD",
          "76": "PA14",
          "77": "PA15",
          "78": "PC10",
          "79": "PC11",
          "80": "PC12",
          "81": "PD0",
          "82": "PD1",
          "83": "PD2",
          "84": "PD3",
          "85": "PD4",
          "86": "PD5",
          "87": "PD6",
          "88": "PD7",
          "89": "PB3",
          "90": "PB4",
          "91": "PB5",
          "92": "PB6",
          "93": "PB7",
          "94": "BOOT0",
          "95": "PB8",
          "96": "PB9",
          "97": "PE0",
          "98": "PE1",
          "99": "VSS",
          "100": "VDD"
        },
        dnp: false
      },
      {
        ref: "C_VDD11",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 850,
        y: 180,
        notes: "Place directly at U_MCU pin 11.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VDD19",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 1020,
        y: 180,
        notes: "Place directly at U_MCU pin 19.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VDD28",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 1190,
        y: 180,
        notes: "Place directly at U_MCU pin 28.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VDD50",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 850,
        y: 330,
        notes: "Place directly at U_MCU pin 50.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VDD75",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 1020,
        y: 330,
        notes: "Place directly at U_MCU pin 75.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VDD100",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 1190,
        y: 330,
        notes: "Place directly at U_MCU pin 100.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_MCU_BULK",
        part: "10u",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 850,
        y: 500,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VBAT",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 1030,
        y: 500,
        notes: "VBAT tied to 3V3; no battery backup.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VCAP1",
        part: "2u2",
        sheet: "02_MCU",
        pins: {
          "1": "VCAP1",
          "2": "GND"
        },
        x: 850,
        y: 680,
        notes: "2.2uF low-ESR <=2ohm. NEVER connect VCAP to 3V3.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_VCAP2",
        part: "2u2",
        sheet: "02_MCU",
        pins: {
          "1": "VCAP2",
          "2": "GND"
        },
        x: 1030,
        y: 680,
        notes: "2.2uF low-ESR <=2ohm. NEVER connect VCAP to 3V3.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_AVDD",
        part: "0R",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "VDDA"
        },
        x: 850,
        y: 850,
        notes: "0R feed; footprint can accept ferrite only after current/impedance review.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_AVDD_HF",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "VDDA",
          "2": "GND"
        },
        x: 1030,
        y: 850,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_AVDD_BULK",
        part: "1u",
        sheet: "02_MCU",
        pins: {
          "1": "VDDA",
          "2": "GND"
        },
        x: 1200,
        y: 850,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_BOOT0",
        part: "10k",
        sheet: "02_MCU",
        pins: {
          "1": "BOOT0",
          "2": "GND"
        },
        x: 120,
        y: 1100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_BOOT1",
        part: "10k",
        sheet: "02_MCU",
        pins: {
          "1": "BOOT1",
          "2": "GND"
        },
        x: 300,
        y: 1100,
        notes: "PB2 boot strap low; currently unused by firmware.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_RESET",
        part: "10k",
        sheet: "02_MCU",
        pins: {
          "1": "3V3",
          "2": "NRST"
        },
        x: 480,
        y: 1100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_RESET",
        part: "100n",
        sheet: "02_MCU",
        pins: {
          "1": "NRST",
          "2": "GND"
        },
        x: 660,
        y: 1100,
        notes: "SWD reset access; no optional reset button fitted.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "Y1",
        part: "crystal",
        sheet: "02_MCU",
        pins: {
          "1": "HSE_IN",
          "2": "GND",
          "3": "HSE_OUT",
          "4": "GND"
        },
        x: 1e3,
        y: 1100,
        notes: "8MHz CL12pF ESR80ohm max; 5032. Pads 2/4 case ground.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_HSE_IN",
        part: "18p",
        sheet: "02_MCU",
        pins: {
          "1": "HSE_IN",
          "2": "GND"
        },
        x: 850,
        y: 1260,
        notes: "C1=C2=18pF with estimated Cstray=3pF gives CL=12pF; verify startup/drive.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_HSE_OUT",
        part: "18p",
        sheet: "02_MCU",
        pins: {
          "1": "HSE_OUT",
          "2": "GND"
        },
        x: 1150,
        y: 1260,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "U_CAN",
        part: "can",
        sheet: "03_CAN",
        pins: {
          "1": "CAN_TX",
          "2": "GND",
          "3": "5V",
          "4": "CAN_RX",
          "5": "3V3",
          "6": "CAN_L",
          "7": "CAN_H",
          "8": "CAN_STB"
        },
        x: 360,
        y: 330,
        notes: "SIT1042AQT/3 SOP8; STB LOW normal/high-speed; CAN1 1Mbps.",
        pinNames: {
          "1": "TXD",
          "2": "GND",
          "3": "VCC",
          "4": "RXD",
          "5": "VIO",
          "6": "CANL",
          "7": "CANH",
          "8": "STB"
        },
        dnp: false
      },
      {
        ref: "R_STB",
        part: "0R",
        sheet: "03_CAN",
        pins: {
          "1": "CAN_STB",
          "2": "GND"
        },
        x: 110,
        y: 500,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_CAN_VCC_HF",
        part: "100n",
        sheet: "03_CAN",
        pins: {
          "1": "5V",
          "2": "GND"
        },
        x: 100,
        y: 100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_CAN_VCC",
        part: "1u",
        sheet: "03_CAN",
        pins: {
          "1": "5V",
          "2": "GND"
        },
        x: 280,
        y: 100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_CAN_VIO_HF",
        part: "100n",
        sheet: "03_CAN",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 470,
        y: 100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_CAN_VIO",
        part: "1u",
        sheet: "03_CAN",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 660,
        y: 100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_TERM",
        part: "120R",
        sheet: "03_CAN",
        pins: {
          "1": "CAN_H",
          "2": "CAN_L"
        },
        x: 680,
        y: 300,
        notes: "FIXED 120ohm. Main controller is a physical bus end. No switch/jumper.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "D_CAN_H",
        part: "can_tvs",
        sheet: "03_CAN",
        pins: {
          "1": "CAN_H",
          "2": "GND"
        },
        x: 900,
        y: 180,
        notes: "Bidirectional SMAJ13CA; verify capacitive loading at 1Mbps.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "D_CAN_L",
        part: "can_tvs",
        sheet: "03_CAN",
        pins: {
          "1": "CAN_L",
          "2": "GND"
        },
        x: 900,
        y: 460,
        notes: "Bidirectional SMAJ13CA; no substitution in Rev.A.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_CAN",
        part: "gh3",
        sheet: "03_CAN",
        pins: {
          "1": "GND",
          "2": "CAN_H",
          "3": "CAN_L"
        },
        x: 1160,
        y: 330,
        notes: "JST GH 1.25mm; numbered pin order is harness contract.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_TF",
        part: "tf",
        sheet: "04_STORAGE",
        pins: {
          "1": "SD_D2",
          "2": "SD_D3",
          "3": "SD_CMD",
          "4": "3V3",
          "5": "SD_CLK",
          "6": "GND",
          "7": "SD_D0",
          "8": "SD_D1",
          "9": "SD_DETECT"
        },
        x: 800,
        y: 360,
        notes: "HRO TF-01A: contact Cd=9, closes to grounded shell when inserted. All shell pins GND; library pad audit required.",
        pinNames: {
          "1": "DAT2",
          "2": "DAT3",
          "3": "CMD",
          "4": "VDD",
          "5": "CLK",
          "6": "VSS",
          "7": "DAT0",
          "8": "DAT1",
          "9": "CD"
        },
        dnp: false,
        extraGroundPinsByName: [
          "GND",
          "SHELL",
          "SHIELD",
          "CASE"
        ]
      },
      {
        ref: "R_SD_D0",
        part: "47k",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "SD_D0"
        },
        x: 100,
        y: 100,
        notes: "SD pull-up 47k in 10k..100k range; CLK has none.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SD_D1",
        part: "47k",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "SD_D1"
        },
        x: 290,
        y: 100,
        notes: "SD pull-up 47k in 10k..100k range; CLK has none.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SD_D2",
        part: "47k",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "SD_D2"
        },
        x: 480,
        y: 100,
        notes: "SD pull-up 47k in 10k..100k range; CLK has none.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SD_D3",
        part: "47k",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "SD_D3"
        },
        x: 670,
        y: 100,
        notes: "SD pull-up 47k in 10k..100k range; CLK has none.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SD_CMD",
        part: "47k",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "SD_CMD"
        },
        x: 860,
        y: 100,
        notes: "SD pull-up 47k in 10k..100k range; CLK has none.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SD_CLK",
        part: "33R",
        sheet: "04_STORAGE",
        pins: {
          "1": "SD_CLK_MCU",
          "2": "SD_CLK"
        },
        x: 180,
        y: 360,
        notes: "Place near MCU PC12; 4-bit SDIO currently 8MHz.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SD_DETECT",
        part: "10k",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "SD_DETECT"
        },
        x: 350,
        y: 520,
        notes: "Active-low mechanical detect independent of DAT3.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_SD_HF",
        part: "100n",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 650,
        y: 630,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_SD_BULK",
        part: "10u",
        sheet: "04_STORAGE",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 860,
        y: 630,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_OLED",
        part: "ph4",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "GND",
          "3": "OLED_SCL",
          "4": "OLED_SDA"
        },
        x: 130,
        y: 160,
        notes: "SSD1306 1.3in 128x64, I2C address0x3C; short harness.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SCL",
        part: "4k7",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "OLED_SCL"
        },
        x: 380,
        y: 120,
        notes: "Default fitted; DNP only if module already has suitable strong pull-ups.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SDA",
        part: "4k7",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "OLED_SDA"
        },
        x: 570,
        y: 120,
        notes: "Default fitted; verify 400kHz rise time and total pull-up resistance.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_ENCODER",
        part: "ph3",
        sheet: "05_UI_IO",
        pins: {
          "1": "GND",
          "2": "ENC_A",
          "3": "ENC_B"
        },
        x: 900,
        y: 170,
        notes: "Mechanical encoder A/B only; no push switch.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_ENC_A",
        part: "10k",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "ENC_A"
        },
        x: 1120,
        y: 100,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_ENC_B",
        part: "10k",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "ENC_B"
        },
        x: 1120,
        y: 260,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_FORWARD",
        part: "ph2",
        sheet: "05_UI_IO",
        pins: {
          "1": "BTN_FORWARD",
          "2": "GND"
        },
        x: 180,
        y: 470,
        notes: "Active LOW. SoftStop is NOT hardware E-STOP.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_FORWARD",
        part: "10k",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "BTN_FORWARD"
        },
        x: 180,
        y: 640,
        notes: "Software debounce; no default RC capacitor.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_BACK",
        part: "ph2",
        sheet: "05_UI_IO",
        pins: {
          "1": "BTN_BACK",
          "2": "GND"
        },
        x: 590,
        y: 470,
        notes: "Active LOW. SoftStop is NOT hardware E-STOP.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_BACK",
        part: "10k",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "BTN_BACK"
        },
        x: 590,
        y: 640,
        notes: "Software debounce; no default RC capacitor.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_SOFTSTOP",
        part: "ph2",
        sheet: "05_UI_IO",
        pins: {
          "1": "BTN_SOFTSTOP",
          "2": "GND"
        },
        x: 1e3,
        y: 470,
        notes: "Active LOW. SoftStop is NOT hardware E-STOP.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_SOFTSTOP",
        part: "10k",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "BTN_SOFTSTOP"
        },
        x: 1e3,
        y: 640,
        notes: "Software debounce; no default RC capacitor.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_HX711",
        part: "ph4",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "GND",
          "3": "HX_DOUT",
          "4": "HX_SCK"
        },
        x: 190,
        y: 930,
        notes: "Remote HX711 close to load cell. 3.3V only; DOUT must not be 5V.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_HX_DOUT",
        part: "33R",
        sheet: "05_UI_IO",
        pins: {
          "1": "HX_DOUT",
          "2": "HX_DOUT_MCU"
        },
        x: 530,
        y: 870,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "R_HX_SCK",
        part: "33R",
        sheet: "05_UI_IO",
        pins: {
          "1": "HX_SCK_MCU",
          "2": "HX_SCK"
        },
        x: 530,
        y: 1030,
        notes: "",
        pinNames: {},
        dnp: false
      },
      {
        ref: "C_HX_LOCAL",
        part: "100n",
        sheet: "05_UI_IO",
        pins: {
          "1": "3V3",
          "2": "GND"
        },
        x: 850,
        y: 950,
        notes: "Board-side connector decoupling; remote HX711 requires its own local capacitors.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_UART",
        part: "ph3",
        sheet: "06_DEBUG",
        pins: {
          "1": "GND",
          "2": "UART_TX",
          "3": "UART_RX"
        },
        x: 130,
        y: 170,
        notes: "UART 3V3 TTL 115200; TX/RX named from controller viewpoint.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "J_SWD",
        part: "ph5",
        sheet: "06_DEBUG",
        pins: {
          "1": "3V3",
          "2": "GND",
          "3": "SWDIO",
          "4": "SWCLK",
          "5": "NRST"
        },
        x: 600,
        y: 170,
        notes: "3V3 is target voltage reference, not external power input.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_24V_IN",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "24V_IN"
        },
        x: 100,
        y: 460,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_5V",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "5V"
        },
        x: 305,
        y: 460,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_3V3",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "3V3"
        },
        x: 510,
        y: 460,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_GND",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "GND"
        },
        x: 715,
        y: 460,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_CAN_TX",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "CAN_TX"
        },
        x: 920,
        y: 460,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_CAN_RX",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "CAN_RX"
        },
        x: 1125,
        y: 460,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_CAN_H",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "CAN_H"
        },
        x: 100,
        y: 650,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_CAN_L",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "CAN_L"
        },
        x: 305,
        y: 650,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_UART_TX",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "UART_TX"
        },
        x: 510,
        y: 650,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_UART_RX",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "UART_RX"
        },
        x: 715,
        y: 650,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_NRST",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "NRST"
        },
        x: 920,
        y: 650,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_BOOT0",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "BOOT0"
        },
        x: 1125,
        y: 650,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_SD_CLK",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "SD_CLK"
        },
        x: 100,
        y: 840,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_SWDIO",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "SWDIO"
        },
        x: 305,
        y: 840,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_SWCLK",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "SWCLK"
        },
        x: 510,
        y: 840,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_USB_DM_RESERVED",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "USB_DM_RESERVED"
        },
        x: 715,
        y: 840,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_USB_DP_RESERVED",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "USB_DP_RESERVED"
        },
        x: 920,
        y: 840,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      },
      {
        ref: "TP_BUCK_PG",
        part: "tp",
        sheet: "06_DEBUG",
        pins: {
          "1": "BUCK_PG"
        },
        x: 1125,
        y: 840,
        notes: "Top-access SMT test point; keep high-speed stubs short.",
        pinNames: {},
        dnp: false
      }
    ],
    mountingHoles: [
      {
        ref: "MH1",
        type: "MOUNT_HOLE_M3",
        diameter_mm: 3.2,
        plated: false,
        net: null,
        keepout_diameter_mm: 6,
        xy: null
      },
      {
        ref: "MH2",
        type: "MOUNT_HOLE_M3",
        diameter_mm: 3.2,
        plated: false,
        net: null,
        keepout_diameter_mm: 6,
        xy: null
      },
      {
        ref: "MH3",
        type: "MOUNT_HOLE_M3",
        diameter_mm: 3.2,
        plated: false,
        net: null,
        keepout_diameter_mm: 6,
        xy: null
      },
      {
        ref: "MH4",
        type: "MOUNT_HOLE_M3",
        diameter_mm: 3.2,
        plated: false,
        net: null,
        keepout_diameter_mm: 6,
        xy: null
      }
    ],
    notes: {
      "01_POWER": [
        "LOGIC SUPPLY ONLY | 6S LiPo 22.2V nominal / 25.2V full",
        "CRITICAL LAYOUT AREA: TI LMR38020 400kHz / 15uH / 5.016V",
        "No motor power distribution. Input protection requires upstream branch fuse."
      ],
      "02_MCU": [
        "STM32F407VET6 | HSE 8MHz | SYSCLK 168MHz",
        "6 x VDD100nF; VCAP1/2 each 2.2uF to GND ONLY",
        "Pin map follows arm_6_axes_0.ioc; unused GPIO no-connect."
      ],
      "03_CAN": [
        "Main controller contains fixed 120 ohm termination.",
        "SIT1042AQT/3: VCC5V / VIO3V3; each 100nF + 1uF",
        "SMAJ13CA remains as requested; signal integrity validation pending."
      ],
      "04_STORAGE": [
        "microSD 4-bit SDIO | external 47k pull-ups on CMD and DAT0..3",
        "CLK33R near MCU; no CLK pull-up | PC7 card detect active LOW"
      ],
      "05_UI_IO": [
        "All external logic 3.3V. Follow frozen connector pin numbers.",
        "SoftStop is software-controlled stop. Hardware E-STOP is external."
      ],
      "06_DEBUG": [
        "UART 3V3 TTL | SWD target reference 3V3",
        "4 x MOUNT_HOLE_M3, 3.2mm NPTH, no net/copper, 6mm keepout",
        "PCB NOT STARTED. Mounting-hole X/Y frozen next revision stage."
      ]
    },
    parts: {
      mcu: {
        lcsc: "C28730",
        mpn: "STM32F407VET6",
        manufacturer: "ST",
        package: "LQFP-100(14x14)",
        category: "Microcontrollers",
        value: "STM32F407VET6",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/eded9b3eda904d8bb3a84cf2887d6a5f.pdf?productCode=C28730",
        productUrl: "https://www.lcsc.com/product-detail/C28730.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 104,
        stockObservedAt: "2026-09-18T16:05:17.398725+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "DAC (Bit)": "12bit",
          "ADC (Bit)": "12bit",
          "Number of I/O": "82",
          "Operating Temperature": "-40\u2103~+85\u2103",
          "Voltage - Supply": "1.8V~3.6V",
          "Program Memory Type": "FLASH",
          EEPROM: "-",
          "Program Storage Size": "512KB",
          "CPU Core": "ARM Cortex-M4",
          "Core Size": "32 Bit",
          "CPU Maximum Speed": "168MHz",
          "Oscillator Type": "Built-in"
        },
        footprint: "LQFP-100_L14.0-W14.0-P0.50-LS16.0-BL [UNVERIFIED]"
      },
      buck: {
        lcsc: "C5149193",
        mpn: "LMR38020FDDAR",
        manufacturer: "TI",
        package: "SO-PowerPAD-8",
        category: "Voltage Regulators - DC DC Switching Regulators",
        value: "5V 2A Buck",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/ec6587c13899eda138625fa2c78da98a.pdf?productCode=C5149193",
        productUrl: "https://www.lcsc.com/product-detail/C5149193.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 5301,
        stockObservedAt: "2026-09-18T16:05:17.410606+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Operating Temperature": "-40\u2103~+150\u2103@(TJ)",
          "Frequency - Switching": "525kHz",
          Function: "Step-down type",
          "Number of Outputs": "1",
          Features: "Cycle-by-cycle current limiting;Hiccup current limiting;Frequency synchronization;EMI optimization (spread spectrum);Light load efficient mode;Operating status indication;Overcurrent Protection",
          "Output Type": "Adjustable",
          Topology: "Buck",
          "Operating Voltage": "4.2V~80V",
          "Switch tube (built-in/external)": "Built-in",
          "Output Voltage": "1V~75V",
          "Output Current": "2A",
          "Quiescent Current": "40uA"
        },
        footprint: "ESOP-8_L4.9-W3.9-P1.27-LS6.0-BL-EP-1 [UNVERIFIED]"
      },
      ldo: {
        lcsc: "C4943338",
        mpn: "AP7361C-33SP-13",
        manufacturer: "DIODES",
        package: "ESOP-8",
        category: "Voltage Regulators - Linear, Low Drop Out (LDO) Regulators",
        value: "3V3 LDO",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/ede62dc0e0b0cdea60995fdce24a9165.pdf?productCode=C4943338",
        productUrl: "https://www.lcsc.com/product-detail/C4943338.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 2120,
        stockObservedAt: "2026-09-18T16:05:17.424000+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Output Voltage": "3.3V",
          "Number of Outputs": "1",
          "Operating Temperature": "-40\u2103~+85\u2103@(Ta)",
          "Supply Current (Iq)": "60uA",
          "Output Configuration": "Positive",
          "Operating Voltage": "6V",
          "Output Current": "1A",
          "Output Type": "Fixed",
          Features: "Enable control;Over Current Protection;Short Circuit Protection;Thermal shutdown"
        },
        footprint: "ESOP-8_L4.9-W3.9-P1.27-LS6.0-BL-EP [UNVERIFIED]"
      },
      can: {
        lcsc: "C5382542",
        mpn: "SIT1042AQT/3",
        manufacturer: "SIT",
        package: "SOP-8",
        category: "Drivers, Receivers, Transceivers",
        value: "CAN 5V / 3V3",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/54c3cce6c41d58bd4bf29d90c12accc3.pdf?productCode=C5382542",
        productUrl: "https://www.lcsc.com/product-detail/C5382542.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 0,
        stockObservedAt: "2026-09-18T16:05:18.651334+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Voltage - Supply": "4.5V~5.5V",
          Type: "CAN FD Transceiver",
          "number of channels": "1",
          "Data Rate": "5Mbps",
          "Operating Temperature": "-40\u2103~+150\u2103",
          Features: "Low-power mode;Wake-up function;Short-Circuit  Protection;Thermal shutdown protection;Undervoltage Protection;TXD dominant timeout;ESD protection;Enable",
          "Supply Current": "70mA",
          "Quiescent Supply Current": "5uA"
        },
        footprint: "SOIC-8_L4.9-W3.9-P1.27-LS6.0-BL [UNVERIFIED]"
      },
      tf: {
        lcsc: "C91145",
        mpn: "TF-01A",
        manufacturer: "Korean Hroparts Elec",
        package: "SMD",
        category: "PC Card Sockets",
        value: "microSD + CD",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/7650eac21a54674b08ad4ba21e17fceb.pdf?productCode=C91145",
        productUrl: "https://www.lcsc.com/product-detail/C91145.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 211555,
        stockObservedAt: "2026-09-18T16:05:17.233163+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Operating Temperature": "-",
          "Card Connection Mode": "Push-Push",
          "Height Above Board": "1.85mm",
          "Supplementary Features": "With Locating Pins",
          "Card Detection": "-",
          "Connector Type": "Connector and Ejector",
          "Card Type": "MicroSD Card (TF Card)"
        },
        footprint: "TF-SMD_TF-01A [UNVERIFIED]"
      },
      xt30: {
        lcsc: "C431092",
        mpn: "XT30PW-M30.G.Y",
        manufacturer: "Changzhou Amass Elec",
        package: "-",
        category: "Power Connectors",
        value: "XT30 2P",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/f40c503d729038bd14858056d73ccb74.pdf?productCode=C431092",
        productUrl: "https://www.lcsc.com/product-detail/C431092.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 81966,
        stockObservedAt: "2026-09-18T16:05:17.371973+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Size/Specification": "2pin",
          "contact resistance": "1.2m\u03A9",
          "Wire gauge": "16/18/20AWG",
          "Rated current": "15A",
          "Connector type": "Model aircraft plug",
          "interface form": "Male",
          "Rated voltage": "500VDC"
        },
        footprint: "CONN-TH_XT30PW-M [UNVERIFIED]"
      },
      ph2: {
        lcsc: "C295747",
        mpn: "S2B-PH-SM4-TB(LF)(SN)",
        manufacturer: "JST",
        package: "SMD,P=2mm,Surface Mount,Right Angle",
        category: "Headers, Male Pins",
        value: "PH2.0 2P",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/2474e9261ed7c5f86b55d373e724589d.pdf?productCode=C295747",
        productUrl: "https://www.lcsc.com/product-detail/C295747.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 27940,
        stockObservedAt: "2026-09-18T16:06:56.470794+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Connector Type": "Surface Mount, Right Angle",
          "Reference Series": "PH",
          "Voltage Rating (Max)": "100V",
          "Contact Material": "Copper alloy",
          "Number of PINs Per Row": "2",
          "Number of Rows": "1",
          Pitch: "2mm",
          "Flame Retardant Rating": "UL94V-0",
          "Current Rating": "2A",
          "Supplementary Features": "Auxiliary Solder Pin",
          "Number of PINs": "2P",
          "Contact Plating": "Tin",
          "Operating Temperature": "-25\u2103~+85\u2103",
          "Plastic Material": "PA",
          Color: "Beige",
          "X-Length of Bottom Edge on Board (Spacing Line)": "7.9mm",
          "Row Spacing": "-",
          "Pin Structure": "1x2P"
        },
        footprint: "CONN-SMD_P2.00_S2B-PH-SM4-TB-LF-SN [UNVERIFIED]"
      },
      ph3: {
        lcsc: "C265101",
        mpn: "S3B-PH-SM4-TB(LF)(SN)",
        manufacturer: "JST",
        package: "SMD,P=2mm,Surface Mount,Right Angle",
        category: "Headers, Male Pins",
        value: "PH2.0 3P",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/6c6fce4367275d84ec6a99bf09f1250f.pdf?productCode=C265101",
        productUrl: "https://www.lcsc.com/product-detail/C265101.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 325,
        stockObservedAt: "2026-09-18T16:06:56.470262+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Connector Type": "Surface Mount, Right Angle",
          "Reference Series": "PH",
          "Voltage Rating (Max)": "100V",
          "Contact Material": "Copper alloy",
          "Number of PINs Per Row": "3",
          "Number of Rows": "1",
          Pitch: "2mm",
          "Flame Retardant Rating": "UL94V-0",
          "Current Rating": "2A",
          "Supplementary Features": "Auxiliary Solder Pin",
          "Number of PINs": "3P",
          "Operating Temperature": "-25\u2103~+85\u2103",
          "Contact Plating": "Tin",
          "Plastic Material": "PA",
          Color: "Beige",
          "X-Length of Bottom Edge on Board (Spacing Line)": "9.9mm",
          "Row Spacing": "-",
          "Pin Structure": "1x3P"
        },
        footprint: "CONN-SMD_S3B-PH-SM4-TB-LF-SN [UNVERIFIED]"
      },
      ph4: {
        lcsc: "C265102",
        mpn: "S4B-PH-SM4-TB(LF)(SN)",
        manufacturer: "JST",
        package: "SMD,P=2mm,Surface Mount,Right Angle",
        category: "Headers, Male Pins",
        value: "PH2.0 4P",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/6c6fce4367275d84ec6a99bf09f1250f.pdf?productCode=C265102",
        productUrl: "https://www.lcsc.com/product-detail/C265102.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 31785,
        stockObservedAt: "2026-09-18T16:06:56.454611+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Connector Type": "Surface Mount, Right Angle",
          "Reference Series": "PH",
          "Voltage Rating (Max)": "100V",
          "Contact Material": "Copper alloy",
          "Number of PINs Per Row": "4",
          "Number of Rows": "1",
          Pitch: "2mm",
          "Flame Retardant Rating": "UL94V-0",
          "Current Rating": "2A",
          "Supplementary Features": "Auxiliary Solder Pin",
          "Number of PINs": "4P",
          "Operating Temperature": "-25\u2103~+85\u2103",
          "Contact Plating": "Tin",
          "Plastic Material": "PA",
          Color: "Beige",
          "X-Length of Bottom Edge on Board (Spacing Line)": "11.9mm",
          "Row Spacing": "-",
          "Pin Structure": "1x4P"
        },
        footprint: "CONN-SMD_S4B-PH-SM4-TB-LF-SN [UNVERIFIED]"
      },
      ph5: {
        lcsc: "C265104",
        mpn: "S5B-PH-SM4-TB(LF)(SN)",
        manufacturer: "JST",
        package: "SMD,P=2mm,Surface Mount,Right Angle",
        category: "Headers, Male Pins",
        value: "PH2.0 5P",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/d87682fbfaf1a1bc6a21af6a6a17fd27.pdf?productCode=C265104",
        productUrl: "https://www.lcsc.com/product-detail/C265104.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 13010,
        stockObservedAt: "2026-09-18T16:06:56.478323+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Connector Type": "Surface Mount, Right Angle",
          "Reference Series": "PH",
          "Voltage Rating (Max)": "100V",
          "Contact Material": "Copper alloy",
          "Number of PINs Per Row": "5",
          "Number of Rows": "1",
          Pitch: "2mm",
          "Flame Retardant Rating": "UL94V-0",
          "Current Rating": "2A",
          "Supplementary Features": "Auxiliary Solder Pin",
          "Number of PINs": "5P",
          "Operating Temperature": "-25\u2103~+85\u2103",
          "Contact Plating": "Tin",
          "Plastic Material": "PA",
          Color: "Beige",
          "X-Length of Bottom Edge on Board (Spacing Line)": "13.9mm",
          "Row Spacing": "-",
          "Pin Structure": "1x5P"
        },
        footprint: "CONN-SMD_S5B-PH-SM4-TB-LF-SN [UNVERIFIED]"
      },
      gh3: {
        lcsc: "C514175",
        mpn: "SM03B-GHS-TB(LF)(SN)",
        manufacturer: "JST",
        package: "SMD,P=1.25mm,Surface Mount,Right Angle",
        category: "Headers, Male Pins",
        value: "GH1.25 3P",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/895c7a62b0771fc9d108814674460a56.pdf?productCode=C514175",
        productUrl: "https://www.lcsc.com/product-detail/C514175.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 0,
        stockObservedAt: "2026-09-18T16:06:57.863693+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Connector Type": "Surface Mount, Right Angle",
          "Reference Series": "GH",
          "Voltage Rating (Max)": "50V",
          "Contact Material": "Phosphor bronze",
          "Number of PINs Per Row": "3",
          "Number of Rows": "1",
          Pitch: "1.25mm",
          "Flame Retardant Rating": "UL94V-0",
          "Current Rating": "1A",
          "Supplementary Features": "Auxiliary Solder Pin",
          "Number of PINs": "3P",
          "Contact Plating": "Tin",
          "Operating Temperature": "-25\u2103~+85\u2103",
          "Plastic Material": "PA9T",
          Color: "White",
          "X-Length of Bottom Edge on Board (Spacing Line)": "7mm",
          "Row Spacing": "-",
          "Pin Structure": "1x3P"
        },
        footprint: "CONN-TH_SM03B-GHS-TB-LF-SN [UNVERIFIED]"
      },
      crystal: {
        lcsc: "C5181482",
        mpn: "5032 8M 12PF 20PPM 4P",
        manufacturer: "SOSET",
        package: "SMD5032-4P",
        category: "Crystals",
        value: "8MHz CL12pF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/4ede9566961a29c7940ef999c46fe124.pdf?productCode=C5181482",
        productUrl: "https://www.lcsc.com/product-detail/C5181482.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 900,
        stockObservedAt: "2026-09-18T16:06:57.805221+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Frequency Stability": "\xB130ppm",
          "Operating Temperature": "-40\u2103~+85\u2103",
          "Load Capacitance": "12pF",
          "Normal temperature Frequency Tolerance": "\xB120ppm",
          Frequency: "8MHz"
        },
        footprint: "CRYSTAL-SMD_4P-L5.0-W3.2-BL_ECS [UNVERIFIED]"
      },
      fuse: {
        lcsc: "C3099",
        mpn: "0451001.MRL",
        manufacturer: "Littelfuse",
        package: "2410",
        category: "Fuses",
        value: "1A 125V",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/00c9748756ca418792b5b22a2bd74996.pdf?productCode=C3099",
        productUrl: "https://www.lcsc.com/product-detail/C3099.html",
        voltage: "125V",
        tolerance: "",
        power: "",
        stockObserved: 12145,
        stockObservedAt: "2026-09-18T16:06:57.955193+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Melting I2t": "0.6029",
          "Interrupt Rating": "50A",
          "Operating Temperature": "-55\u2103~+125\u2103",
          Height: "-",
          Width: "2.5mm",
          Type: "Surface Mount Fuse",
          Length: "6.1mm",
          "Current Rating": "1A",
          "Voltage Rating (DC)": "125V"
        },
        footprint: "FUSE-SMD_L6.1-W2.6 [UNVERIFIED]"
      },
      reverse: {
        lcsc: "C2848687",
        mpn: "SS310",
        manufacturer: "GOODWORK",
        package: "SMA(DO-214AC)",
        category: "Single Diodes",
        value: "SS310 100V 3A",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/99efa47dcb98c002cc80beb74904af65.pdf?productCode=C2848687",
        productUrl: "https://www.lcsc.com/product-detail/C2848687.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 296980,
        stockObservedAt: "2026-09-18T16:06:57.936266+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Operating Junction Temperature Range": "-65\u2103~+150\u2103",
          "Voltage - DC Reverse (Vr) (Max)": "100V",
          "Diode Configuration": "Standalone",
          "Voltage - Forward(Vf@If)": "850mV@3A",
          "Reverse Leakage Current (Ir)": "20uA",
          "Non-Repetitive Peak Forward Surge Current": "80A",
          "Current - Rectified": "3A"
        },
        footprint: "SMA_L4.3-W2.6-LS5.1-R-RD [UNVERIFIED]"
      },
      input_tvs: {
        lcsc: "C173526",
        mpn: "SMBJ33A",
        manufacturer: "MDD(Microdiode Semiconductor)",
        package: "DO-214AA(SMB)",
        category: "TVS Diodes",
        value: "SMBJ33A",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/2fdb55289569ebfc0bc21eab3853afe6.pdf?productCode=C173526",
        productUrl: "https://www.lcsc.com/product-detail/C173526.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 38360,
        stockObservedAt: "2026-09-18T16:06:57.945141+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Clamping Voltage": "53.3V",
          "Operating Temperature": "-65\u2103~+150\u2103",
          "Peak Pulse Current (Ipp)": "11.3A@10/1000us",
          "Peak Pulse Power Dissipation (Ppp)": "600W@10/1000us",
          "Number of Channels": "1",
          "Voltage - Breakdown": "40.6V",
          type: "TVS",
          Polarity: "Unidirectional",
          "Reverse Leakage Current (Ir)": "1uA",
          "Reverse Stand-Off Voltage (Vrwm)": "33V"
        },
        footprint: "SMB_L4.6-W3.6-LS5.3-RD [UNVERIFIED]"
      },
      can_tvs: {
        lcsc: "C134951",
        mpn: "SMAJ13CA-13-F",
        manufacturer: "DIODES",
        package: "SMA",
        category: "TVS Diodes",
        value: "SMAJ13CA",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/3c9d77b4a0a6f95a3a2d0836916a6837.pdf?productCode=C134951",
        productUrl: "https://www.lcsc.com/product-detail/C134951.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 10,
        stockObservedAt: "2026-09-18T16:06:57.961725+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Clamping Voltage": "21.5V",
          "Operating Temperature": "-55\u2103~+150\u2103",
          "Peak Pulse Current (Ipp)": "18.6A",
          "Peak Pulse Power Dissipation (Ppp)": "400W",
          "Number of Channels": "1",
          "Voltage - Breakdown": "15.9V",
          type: "TVS",
          "Reverse Leakage Current (Ir)": "5uA",
          Polarity: "Bidirectional",
          "Reverse Stand-Off Voltage (Vrwm)": "13V"
        },
        footprint: "SMA_L4.4-W2.6-LS5.0-BI [UNVERIFIED]"
      },
      inductor: {
        lcsc: "C40000",
        mpn: "SMDRI127-150MT",
        manufacturer: "Shun Xiang Nuo",
        package: "SMD,12.3x12.3mm",
        category: "Fixed Inductors",
        value: "15uH",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/aa59274d1174eb16f1491f7db2812ea7.pdf?productCode=C40000",
        productUrl: "https://www.lcsc.com/product-detail/C40000.html",
        voltage: "",
        tolerance: "\xB120%",
        power: "",
        stockObserved: 10660,
        stockObservedAt: "2026-09-18T16:06:56.440345+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Current Rating": "4.5A",
          Inductance: "15uH",
          Tolerance: "\xB120%",
          "Current - Saturation(Isat)": "8A",
          "DC Resistance(DCR)": "27m\u03A9",
          Type: "Magnetic Shielded Inductor"
        },
        footprint: "IND-SMD_L12.5-W12.5_RLF12545T [UNVERIFIED]"
      },
      led: {
        lcsc: "C205441",
        mpn: "ORH-B35A",
        manufacturer: "Orient",
        package: "0805",
        category: "LED Indication - Discrete",
        value: "BLUE",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/4ccd793e0537a8e53b7b3f15c5c9c51e.pdf?productCode=C205441",
        productUrl: "https://www.lcsc.com/product-detail/C205441.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 136250,
        stockObservedAt: "2026-09-18T16:06:56.406478+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Installation method": "-",
          "Color Temperature": "-",
          "Peak Wavelength": "465nm",
          "Lens Color": "Water Clear",
          "Luminous Intensity": "20mcd~80mcd",
          "Viewing Angle": "130\xB0",
          "Forward Current": "5mA",
          "Illumination Color": "Blue",
          "Lamp Holder Type": "-",
          "Operating Temperature": "-25\u2103~+80\u2103",
          "Voltage - Forward(Vf)": "3.2V",
          Wavelength: "459nm~468nm",
          "Diode Configuration": "Discrete Diode"
        },
        footprint: "LED0805-R-RD [UNVERIFIED]"
      },
      tp: {
        lcsc: "C2906768",
        mpn: "5015",
        manufacturer: "Keystone",
        package: "-",
        category: "Test Points",
        value: "SMT test point",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/92094172d6d10050dfee88ee891cc96e.pdf?productCode=C2906768",
        productUrl: "https://www.lcsc.com/product-detail/C2906768.html",
        voltage: "",
        tolerance: "",
        power: "",
        stockObserved: 0,
        stockObservedAt: "2026-09-18T16:12:22.567618+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Connector Type": "Ferrite Ring (SMD)",
          "Plastic Color": "Non-Plastic",
          "Board Space (Diameter \u03C6/ Length x Width)": "3.43mmx1.78mm"
        },
        footprint: "TEST-SMD_L2.7-W1.0 [UNVERIFIED]"
      },
      "100n": {
        lcsc: "C1711",
        mpn: "CL21B104KBCNNNC",
        manufacturer: "Samsung Electro-Mechanics",
        package: "0805",
        category: "Ceramic Capacitors",
        value: "100nF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C1711",
        productUrl: "https://www.lcsc.com/product-detail/C1711.html",
        voltage: "50V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 215460,
        stockObservedAt: "2026-09-18T16:08:24.796721+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "100nF",
          "Temperature Coefficient": "X7R",
          Tolerance: "\xB110%",
          "Voltage Rating": "50V"
        },
        footprint: "C0805 [UNVERIFIED]"
      },
      "100n100": {
        lcsc: "C398816",
        mpn: "CL21B104KCFSFNE",
        manufacturer: "Samsung Electro-Mechanics",
        package: "0805",
        category: "Ceramic Capacitors",
        value: "100nF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/027b5a29c43d158ed795bf73acd3fcbc.pdf?productCode=C398816",
        productUrl: "https://www.lcsc.com/product-detail/C398816.html",
        voltage: "100V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 42300,
        stockObservedAt: "2026-09-18T16:06:59.304417+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "100nF",
          "Temperature Coefficient": "X7R",
          Tolerance: "\xB110%",
          "Voltage Rating": "100V"
        },
        footprint: "C0805 [UNVERIFIED]"
      },
      "1u": {
        lcsc: "C28323",
        mpn: "CL21B105KBFNNNE",
        manufacturer: "Samsung Electro-Mechanics",
        package: "0805",
        category: "Ceramic Capacitors",
        value: "1uF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C28323",
        productUrl: "https://www.lcsc.com/product-detail/C28323.html",
        voltage: "50V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 739490,
        stockObservedAt: "2026-09-18T16:05:15.879703+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "1uF",
          "Temperature Coefficient": "X7R",
          Tolerance: "\xB110%",
          "Voltage Rating": "50V"
        },
        footprint: "C0805 [UNVERIFIED]"
      },
      "2u2": {
        lcsc: "C87994",
        mpn: "CL21A225KAFNNNE",
        manufacturer: "Samsung Electro-Mechanics",
        package: "0805",
        category: "Ceramic Capacitors",
        value: "2.2uF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/cc213578a492448787051d6bc0e87012.pdf?productCode=C87994",
        productUrl: "https://www.lcsc.com/product-detail/C87994.html",
        voltage: "25V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 51600,
        stockObservedAt: "2026-09-18T16:06:59.249840+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "2.2uF",
          "Temperature Coefficient": "X5R",
          Tolerance: "\xB110%",
          "Voltage Rating": "25V"
        },
        footprint: "C0805 [UNVERIFIED]"
      },
      "10u": {
        lcsc: "C15850",
        mpn: "CL21A106KAYNNNE",
        manufacturer: "Samsung Electro-Mechanics",
        package: "0805",
        category: "Ceramic Capacitors",
        value: "10uF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C15850",
        productUrl: "https://www.lcsc.com/product-detail/C15850.html",
        voltage: "25V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 1980940,
        stockObservedAt: "2026-09-18T16:05:15.911253+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "10uF",
          "Temperature Coefficient": "X5R",
          Tolerance: "\xB110%",
          "Voltage Rating": "25V"
        },
        footprint: "C0805 [UNVERIFIED]"
      },
      "22u": {
        lcsc: "C309062",
        mpn: "CL32B226KAJNNNE",
        manufacturer: "Samsung Electro-Mechanics",
        package: "1210",
        category: "Ceramic Capacitors",
        value: "22uF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C309062",
        productUrl: "https://www.lcsc.com/product-detail/C309062.html",
        voltage: "25V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 140056,
        stockObservedAt: "2026-09-18T16:05:16.025634+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "22uF",
          "Temperature Coefficient": "X7R",
          Tolerance: "\xB110%",
          "Voltage Rating": "25V"
        },
        footprint: "C1210 [UNVERIFIED]"
      },
      "4u7HV": {
        lcsc: "C2325876",
        mpn: "12101C475K4T2A",
        manufacturer: "Kyocera AVX",
        package: "1210",
        category: "Ceramic Capacitors",
        value: "4.7uF",
        datasheet: "https://www.lcsc.com/datasheet/C2325876.pdf",
        productUrl: "https://www.lcsc.com/product-detail/C2325876.html",
        voltage: "100V",
        tolerance: "\xB110%",
        power: "",
        stockObserved: 29,
        stockObservedAt: "2026-09-18T16:05:19.158226+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "4.7uF",
          "Temperature Coefficient": "X7R",
          Tolerance: "\xB110%",
          "Voltage Rating": "100V"
        },
        footprint: "C1210 [UNVERIFIED]"
      },
      "47uHV": {
        lcsc: "C178377",
        mpn: "63SXV47M",
        manufacturer: "PANASONIC",
        package: "SMD,D10xL12.6mm",
        category: "Aluminum - Polymer Capacitors",
        value: "47uF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/128402af54cf4e6ebab28b0ce30e07fb.pdf?productCode=C178377",
        productUrl: "https://www.lcsc.com/product-detail/C178377.html",
        voltage: "63V",
        tolerance: "\xB120%",
        power: "",
        stockObserved: 14,
        stockObservedAt: "2026-09-18T16:05:17.353434+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Lifetime: "1000hrs@125\u2103",
          Capacitance: "47uF",
          "Height - Seated (Max)": "12.6mm",
          "Polarity type": "Polarized",
          "Voltage Rating": "63V",
          "Operating Temperature": "-55\u2103~+125\u2103",
          Diameter: "10mm",
          "Pin Spacing": "-",
          "Equivalent Series Resistance(ESR)": "28m\u03A9@100kHz",
          "Ripple Current": "980mA@100kHz",
          Tolerance: "\xB120%"
        },
        footprint: "CAP-SMD_BD10.0-L10.3-W10.3-LS11.0-FD-1 [UNVERIFIED]"
      },
      "18p": {
        lcsc: "C1797",
        mpn: "0805CG180J500NT",
        manufacturer: "FH",
        package: "0805",
        category: "Ceramic Capacitors",
        value: "18pF",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/09fe2daf22580a99fa7fea93ccb20fb7.pdf?productCode=C1797",
        productUrl: "https://www.lcsc.com/product-detail/C1797.html",
        voltage: "50V",
        tolerance: "\xB15%",
        power: "",
        stockObserved: 144700,
        stockObservedAt: "2026-09-18T16:05:18.732469+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          Capacitance: "18pF",
          "Temperature Coefficient": "C0G",
          Tolerance: "\xB15%",
          "Voltage Rating": "50V"
        },
        footprint: "C0805 [UNVERIFIED]"
      },
      "0R": {
        lcsc: "C17477",
        mpn: "0805W8F0000T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "0R",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17477",
        productUrl: "https://www.lcsc.com/product-detail/C17477.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 5719e3,
        stockObservedAt: "2026-09-18T16:05:14.473390+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "0\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1800ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "10k": {
        lcsc: "C17414",
        mpn: "0805W8F1002T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "10k",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17414",
        productUrl: "https://www.lcsc.com/product-detail/C17414.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 45967800,
        stockObservedAt: "2026-09-18T16:05:14.519935+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "10k\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "100k": {
        lcsc: "C149504",
        mpn: "0805W8F1003T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "100k",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C149504",
        productUrl: "https://www.lcsc.com/product-detail/C149504.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 3507600,
        stockObservedAt: "2026-09-18T16:08:25.228279+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "100k\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "64k9": {
        lcsc: "C17788",
        mpn: "0805W8F6492T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "64.9k",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17788",
        productUrl: "https://www.lcsc.com/product-detail/C17788.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 3700,
        stockObservedAt: "2026-09-18T16:05:15.867123+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "64.9k\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "24k9": {
        lcsc: "C17571",
        mpn: "0805W8F2492T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "24.9k",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17571",
        productUrl: "https://www.lcsc.com/product-detail/C17571.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 97100,
        stockObservedAt: "2026-09-18T16:05:15.886149+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "24.9k\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "4k7": {
        lcsc: "C17673",
        mpn: "0805W8F4701T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "4.7k",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17673",
        productUrl: "https://www.lcsc.com/product-detail/C17673.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 3571100,
        stockObservedAt: "2026-09-18T16:08:24.819579+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "4.7k\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "47k": {
        lcsc: "C17713",
        mpn: "0805W8F4702T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "47k",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17713",
        productUrl: "https://www.lcsc.com/product-detail/C17713.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 1484500,
        stockObservedAt: "2026-09-18T16:05:14.480943+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "47k\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "120R": {
        lcsc: "C17437",
        mpn: "0805W8F1200T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "120R",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17437",
        productUrl: "https://www.lcsc.com/product-detail/C17437.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 1364400,
        stockObservedAt: "2026-09-18T16:08:24.969563+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "120\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "33R": {
        lcsc: "C17634",
        mpn: "0805W8F330JT5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "33R",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17634",
        productUrl: "https://www.lcsc.com/product-detail/C17634.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 1487600,
        stockObservedAt: "2026-09-18T16:05:14.489420+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "33\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          Tolerance: "\xB11%",
          "Temperature Coefficient": "\xB1200ppm/\u2103"
        },
        footprint: "R0805 [UNVERIFIED]"
      },
      "470R": {
        lcsc: "C17710",
        mpn: "0805W8F4700T5E",
        manufacturer: "UNI-ROYAL",
        package: "0805",
        category: "Chip Resistor - Surface Mount",
        value: "470R",
        datasheet: "https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17710",
        productUrl: "https://www.lcsc.com/product-detail/C17710.html",
        voltage: "150V",
        tolerance: "\xB11%",
        power: "125mW",
        stockObserved: 2441500,
        stockObservedAt: "2026-09-18T16:08:24.385570+00:00",
        lifecycle: "normal",
        libraryStatus: "OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING",
        parameters: {
          "Power(Watts)": "125mW",
          Resistance: "470\u03A9",
          "Voltage Rating": "150V",
          "Operating Temperature": "-55\u2103~+155\u2103",
          Type: "Thick Film Resistor",
          "Temperature Coefficient": "\xB1100ppm/\u2103",
          Tolerance: "\xB11%"
        },
        footprint: "R0805 [UNVERIFIED]"
      }
    }
  };

  // src/generator.js
  function Design_NormalizeName(value) {
    return String(value ?? "").toUpperCase().replace(/[^A-Z0-9]/g, "");
  }
  function Design_CheckPinName(expected, actual) {
    const e = Design_NormalizeName(expected), a = Design_NormalizeName(actual);
    const aliases = {
      K: ["K", "CATHODE"],
      A: ["A", "ANODE"],
      EP: ["EP", "EPAD", "GND", "PAD", "THERMALPAD"],
      NC: ["NC", "NOCONNECT"],
      CD: ["CD", "DETECT", "CARDDETECT"],
      DAT0: ["DAT0", "DATA0", "D0"],
      DAT1: ["DAT1", "DATA1", "D1"],
      DAT2: ["DAT2", "DATA2", "D2"],
      DAT3: ["DAT3", "DATA3", "D3", "CDDAT3"]
    };
    if (aliases[e]) return aliases[e].includes(a);
    return a === e || a.startsWith(e + "OSC") || a.startsWith(e + "TAMP") || e === "VDD" && /^VDD\d*$/.test(a) || e === "VSS" && /^VSS\d*$/.test(a);
  }
  async function Library_ResolvePart(eda2, part) {
    const libraryUuid = await eda2.lib_LibrariesList.getSystemLibraryUuid();
    if (!libraryUuid) throw new Error("Official system library unavailable. Sign in/connect the official library.");
    const candidates = await eda2.lib_Device.getByLcscIds([part.lcsc], libraryUuid, true);
    const matches = [];
    for (const item of candidates || []) {
      if (item.libraryUuid !== libraryUuid) continue;
      if (item.supplierId !== part.lcsc || Design_NormalizeName(item.manufacturerId) !== Design_NormalizeName(part.mpn)) continue;
      let detail, detailError;
      try {
        detail = await eda2.lib_Device.get(item.uuid, libraryUuid);
      } catch (error) {
        detailError = String(error);
      }
      if (detail && (detail.property?.supplierId !== part.lcsc || detail.association?.footprint?.uuid !== item.footprintUuid || detail.association?.symbol?.uuid !== item.otherProperty?.Symbol)) continue;
      if (!detail) {
        const symbolUuid = item.otherProperty?.Symbol;
        if (!symbolUuid || !item.footprintUuid) continue;
        detail = { ...item, association: {
          symbol: { uuid: symbolUuid, libraryUuid },
          footprint: { uuid: item.footprintUuid, libraryUuid, name: item.footprintName }
        }, metadataSource: "OFFICIAL_SEARCH_ITEM", detailError };
      }
      if (!detail.association?.symbol || !detail.association?.footprint) continue;
      detail.catalogMpn = item.manufacturerId;
      detail.catalogFootprintName = item.footprintName;
      detail.catalogMetadataDiffers = Boolean(detail.property?.manufacturerId && detail.property.manufacturerId !== item.manufacturerId);
      matches.push(detail);
    }
    const unique = [...new Map(matches.map((d) => [d.uuid, d])).values()];
    if (unique.length !== 1) throw new Error(`${part.lcsc} ${part.mpn}: expected one exact official device, found ${unique.length}. No substitute or UUID is guessed.`);
    return unique[0];
  }
  async function Project_Create(eda2, design) {
    const current = await eda2.dmt_Project.getCurrentProjectInfo();
    if (current) throw new Error("Use a new empty EasyEDA window. Existing project is left untouched; openProject can discard unsaved changes.");
    const suffix = (/* @__PURE__ */ new Date()).toISOString().replace(/[^0-9]/g, "");
    const projectUuid = await eda2.dmt_Project.createProject(`${design.name}_${suffix}`, `arm-six-axes-reva-${suffix}`);
    if (!projectUuid) throw new Error("createProject returned no UUID. Check online/half-offline mode and project creation permission.");
    if ((await eda2.dmt_Project.getCurrentProjectInfo())?.uuid !== projectUuid && !await eda2.dmt_Project.openProject(projectUuid)) throw new Error(`Project ${projectUuid} created but openProject failed. Inspect it before any retry.`);
    return await Project_PrepareSchematic(eda2, design, projectUuid);
  }
  async function Project_PrepareSchematic(eda2, design, projectUuid) {
    const active = await eda2.dmt_Project.getCurrentProjectInfo();
    if (active?.uuid !== projectUuid) throw new Error("New project was not activated.");
    if (!active.friendlyName?.startsWith(design.name + "_")) throw new Error("Not a project created by this generator.");
    if ((await eda2.dmt_Schematic.getAllSchematicPagesInfo()).length) throw new Error("Project already has schematic pages. Inspect before recovery.");
    const schematicUuid = await eda2.dmt_Schematic.createSchematic();
    if (!schematicUuid) throw new Error("Cannot create schematic.");
    await eda2.dmt_Schematic.modifySchematicName(schematicUuid, design.name);
    const existing = (await eda2.dmt_Schematic.getAllSchematicPagesInfo()).filter((p) => p.parentSchematicUuid === schematicUuid);
    const pages = {};
    for (let i = 0; i < design.sheets.length; i++) {
      const uuid = existing[i]?.uuid || await eda2.dmt_Schematic.createSchematicPage(schematicUuid);
      if (!uuid || !await eda2.dmt_Schematic.modifySchematicPageName(uuid, design.sheets[i])) throw new Error("Cannot create/name schematic page.");
      pages[design.sheets[i]] = uuid;
    }
    return { projectUuid, schematicUuid, pages };
  }
  async function Sheet_Open(eda2, project, sheet) {
    const active = await eda2.dmt_Project.getCurrentProjectInfo();
    if (active?.uuid !== project.projectUuid) throw new Error("Project changed. Stop without touching the current project.");
    const pageUuid = project.pages[sheet];
    const tab = await eda2.dmt_EditorControl.openDocument(pageUuid);
    if (!tab || !await eda2.dmt_EditorControl.activateDocument(tab)) throw new Error(`Cannot activate ${sheet}`);
    const page = await eda2.dmt_Schematic.getCurrentSchematicPageInfo();
    if (page?.uuid !== pageUuid) throw new Error("Wrong active schematic page.");
  }
  async function Sheet_AddNotes(eda2, design, sheet) {
    await eda2.sch_PrimitiveText.create(30, -70, `${design.name} / ${sheet}`, 0, null, null, 18, true);
    for (const [i, note] of design.notes[sheet].entries()) {
      await eda2.sch_PrimitiveText.create(30, -40 + i * 18, note, 0, null, null, 10);
    }
  }
  async function Component_Place(eda2, component, part, device) {
    if ((device.subPartNames || []).length > 1) throw new Error(`${component.ref}: multi-part library symbol requires explicit reviewed placement; not flattened.`);
    const primitive = await eda2.sch_PrimitiveComponent.create(
      device,
      component.x,
      component.y,
      device.subPartNames?.[0] || "",
      0,
      false,
      !component.dnp,
      true
    );
    if (!primitive) throw new Error(`Cannot place ${component.ref}`);
    const id = primitive.getState_PrimitiveId();
    const changed = await eda2.sch_PrimitiveComponent.modify(id, {
      designator: component.ref,
      name: part.value,
      manufacturer: part.manufacturer,
      manufacturerId: part.mpn,
      supplier: "LCSC",
      supplierId: part.lcsc,
      otherProperty: { Datasheet: part.datasheet, Notes: component.notes, DNP: component.dnp ? "YES" : "NO", Revision: "A" }
    });
    if (!changed) throw new Error(`Cannot set ${component.ref} properties.`);
    const pinObjects = await eda2.sch_PrimitiveComponent.getAllPinsByPrimitiveId(id);
    if (!pinObjects?.length) throw new Error(`${component.ref}: no symbol pins.`);
    const pins = pinObjects.map((p) => ({
      number: p.getState_PinNumber(),
      name: p.getState_PinName(),
      x: p.getState_X(),
      y: p.getState_Y(),
      rotation: p.getState_Rotation(),
      id: p.getState_PrimitiveId()
    }));
    const connected = [];
    for (const pin of pins) {
      if (!Object.hasOwn(component.pins, pin.number)) {
        const shell = component.extraGroundPinsByName?.includes(Design_NormalizeName(pin.name));
        if (!shell) throw new Error(`${component.ref}: unexpected pin ${pin.number} (${pin.name}). Must be reviewed before wiring.`);
        pin.net = "GND";
      } else pin.net = component.pins[pin.number];
      const expected = component.pinNames[pin.number];
      if (expected && !Design_CheckPinName(expected, pin.name)) throw new Error(`${component.ref}.${pin.number}: datasheet=${expected}, library=${pin.name}; not wired.`);
      connected.push(pin);
    }
    for (const number of Object.keys(component.pins)) {
      if (!pins.some((p) => p.number === number)) throw new Error(`${component.ref}: missing pin ${number}, including mandatory exposed pads.`);
    }
    if (component.part === "tf" && !pins.some((p) => !Object.hasOwn(component.pins, p.number) && p.net === "GND")) {
      throw new Error("TF socket shell missing from symbol. Card detect return and shell pads require review.");
    }
    return {
      ref: component.ref,
      id,
      pins: connected,
      deviceUuid: device.uuid,
      libraryUuid: device.libraryUuid,
      association: device.association
    };
  }
  async function Component_WirePins(eda2, placed, offset = 0, count = 10) {
    const objects = await eda2.sch_PrimitiveComponent.getAllPinsByPrimitiveId(placed.id);
    if (!objects) throw new Error("Component pins disappeared.");
    const xs = placed.pins.map((p) => p.x), ys = placed.pins.map((p) => p.y);
    const midX = (Math.min(...xs) + Math.max(...xs)) / 2;
    const midY = (Math.min(...ys) + Math.max(...ys)) / 2;
    const records = [];
    for (const pin of placed.pins.slice(offset, offset + count)) {
      const object = objects.find((p) => p.getState_PrimitiveId() === pin.id);
      if (!object) throw new Error("Pin identity changed.");
      if (pin.net === null) {
        if (!await eda2.sch_PrimitivePin.modify(object, { noConnected: true })) throw new Error("Cannot set no-connect.");
        records.push({ pin: pin.number, net: null });
        continue;
      }
      const horizontal = Math.abs(Math.cos(pin.rotation * Math.PI / 180)) > 0.5;
      const dx = horizontal ? pin.x <= midX ? -25 : 25 : 0;
      const dy = horizontal ? 0 : pin.y <= midY ? -25 : 25;
      const endX = pin.x + dx, endY = pin.y + dy;
      const wire = await eda2.sch_PrimitiveWire.create([pin.x, pin.y, endX, endY], pin.net);
      const port = await eda2.sch_PrimitiveComponent.createNetPort(
        "BI",
        pin.net,
        endX,
        endY,
        horizontal ? dx < 0 ? 180 : 0 : dy < 0 ? 270 : 90,
        false
      );
      if (!wire || !port) throw new Error(`Wire/net port failed for ${placed.ref}.${pin.number}`);
      records.push({ pin: pin.number, net: pin.net });
    }
    return records;
  }
  function Footprint_ReadPads(source) {
    const pads = [];
    for (const line of source.split(/\r?\n/)) {
      if (!line.trim()) continue;
      const split = line.indexOf("||");
      if (split < 0) continue;
      const header = JSON.parse(line.slice(0, split));
      if (header.type !== "PAD") continue;
      const data = JSON.parse(line.slice(split + 2).replace(/\|\s*$/, ""));
      pads.push({
        number: String(data.num),
        x: data.centerX,
        y: data.centerY,
        hole: data.hole,
        plated: data.plated,
        shape: data.defaultPad
      });
    }
    if (!pads.length) throw new Error("No pads decoded: unsupported footprint document format; manual review required.");
    return pads;
  }
  async function Sheet_AuditAndSave(eda2, placed) {
    const sources = await eda2.sys_FileManager.getDocumentFootprintSources();
    const audits = [];
    for (const component of placed) {
      const footprint = component.association.footprint;
      const source = sources.find((s) => s.footprintUuid === footprint.uuid);
      if (!source) throw new Error(`${component.ref}: footprint source absent.`);
      const pads = Footprint_ReadPads(source.documentSource);
      for (const pin of component.pins) {
        if (!pads.some((p) => p.number === pin.number)) throw new Error(`${component.ref}: symbol pin ${pin.number} has no footprint pad.`);
      }
      const extraPads = pads.filter((p) => !component.pins.some((pin) => pin.number === p.number));
      if (extraPads.some((p) => p.number && p.number !== "0")) throw new Error(`${component.ref}: extra numbered footprint pads require explicit mechanical/electrical review.`);
      audits.push({
        ref: component.ref,
        footprintUuid: footprint.uuid,
        pads,
        status: "PIN_NUMBERS_MATCH; DIMENSIONS_AND_POLARITY_REVIEW_REQUIRED"
      });
    }
    if (!await eda2.sch_Document.save()) throw new Error("Schematic save failed.");
    await eda2.dmt_EditorControl.zoomToAllPrimitives();
    return audits;
  }
  async function Sheet_CheckERC(eda2) {
    const errors = await eda2.sch_Drc.check(true, false, true);
    if (typeof errors === "boolean") return {
      executed: true,
      errors: null,
      passed: errors,
      detailAvailable: false,
      status: errors ? "LEGACY_CHECK_PASSED_NO_DETAILS" : "LEGACY_CHECK_FAILED_NO_DETAILS"
    };
    if (!Array.isArray(errors)) throw new Error("Unexpected ERC response; do not report PASS.");
    return { executed: true, errors, detailAvailable: true, status: errors.length ? "ISSUES_REPORTED" : "NO_ISSUES_REPORTED" };
  }
  async function Project_Export(eda2, outputPath) {
    const file = await eda2.sys_FileManager.getProjectFile("arm_6_axes_0_revA.epro", void 0, "epro");
    if (!file) throw new Error("Editable project export failed.");
    if (outputPath) {
      if (!await eda2.sys_FileSystem.saveFileToFileSystem(outputPath, file, void 0, false)) throw new Error("Export path exists or is not writable; no file overwritten.");
    } else await eda2.sys_FileSystem.saveFile(file, "arm_6_axes_0_revA.epro");
    return { editableProjectExported: true, size: file.size };
  }
  async function Design_Generate(eda2, design) {
    const devices = {};
    for (const [key, part] of Object.entries(design.parts)) devices[key] = await Library_ResolvePart(eda2, part);
    const project = await Project_Create(eda2, design);
    const report = { project, parts: Object.fromEntries(Object.entries(devices).map(([k, d]) => [k, { uuid: d.uuid, libraryUuid: d.libraryUuid, association: d.association }])), sheets: {}, pcb: "NOT_STARTED" };
    for (const sheet of design.sheets) {
      await Sheet_Open(eda2, project, sheet);
      await Sheet_AddNotes(eda2, design, sheet);
      const placed = [];
      for (const component of design.components.filter((c) => c.sheet === sheet)) {
        const item = await Component_Place(eda2, component, design.parts[component.part], devices[component.part]);
        for (let i = 0; i < item.pins.length; i += 10) await Component_WirePins(eda2, item, i, 10);
        placed.push(item);
      }
      report.sheets[sheet] = { placed, audit: await Sheet_AuditAndSave(eda2, placed), erc: await Sheet_CheckERC(eda2) };
    }
    await eda2.sys_FileSystem.saveFile(new Blob([JSON.stringify(report, null, 2)], { type: "application/json" }), "revA_runtime_audit.json");
    report.export = await Project_Export(eda2);
    return report;
  }

  // src/index.js
  async function generateRevA() {
    try {
      await Design_Generate(eda, controller_revA_default);
    } catch (error) {
      await eda.sys_Dialog.showInformationMessage(`Rev.A generation stopped: ${String(error)}
No ERC PASS is claimed. Any new partial project is retained for diagnosis.`);
      throw error;
    }
  }
  return __toCommonJS(index_exports);
})();
