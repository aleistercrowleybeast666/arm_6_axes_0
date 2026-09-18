"""Electrical intent checks; these are NOT EasyEDA ERC or hardware validation."""
import csv
import json
import re
import unittest
from pathlib import Path
from design import ROOT, build_design, MCU_NAMES

class DesignChecks(unittest.TestCase):
    def setUp(self):
        self.d=build_design();self.c={c['ref']:c for c in self.d['components']}
        self.parts=json.loads((ROOT/'design/parts.json').read_text(encoding='utf-8'))
    def pin(self,ref,pin):return self.c[ref]['pins'][str(pin)]
    def test_unique_complete_component_and_bom_coverage(self):
        self.assertEqual(len(self.c),len(self.d['components']))
        self.assertEqual(set(c['sheet'] for c in self.c.values()),set(self.d['sheets']))
        for c in self.c.values():self.assertIn(c['part'],self.parts)
        with (ROOT/'bom/arm_6_axes_0_revA_bom.csv').open(encoding='utf-8-sig') as f: rows=list(csv.DictReader(f))
        self.assertEqual({r['RefDes'] for r in rows},set(self.c)|{'MH1','MH2','MH3','MH4'})
    def test_ioc_pin_contract(self):
        ioc=(ROOT.parent/'arm_6_axes_0.ioc').read_text(encoding='utf-8-sig')
        expected={'PB8':'CAN1_RX','PB9':'CAN1_TX','PB6':'I2C1_SCL','PB7':'I2C1_SDA','PA6':'S_TIM3_CH1','PA7':'S_TIM3_CH2','PC8':'SDIO_D0','PC9':'SDIO_D1','PC10':'SDIO_D2','PC11':'SDIO_D3','PC12':'SDIO_CK','PD2':'SDIO_CMD','PA9':'USART1_TX','PA10':'USART1_RX','PA13':'SYS_JTMS-SWDIO','PA14':'SYS_JTCK-SWCLK','PE0':'GPXTI0','PE1':'GPIO_Output','PE2':'GPIO_Input','PE3':'GPIO_Input','PE4':'GPXTI4','PC7':'GPIO_Input'}
        for pin,signal in expected.items():self.assertRegex(ioc,re.escape(pin)+r'\.Signal='+re.escape(signal)+r'(?:\r?\n|$)')
        connections={'PB8':'CAN_RX','PB9':'CAN_TX','PB6':'OLED_SCL','PB7':'OLED_SDA','PA6':'ENC_A','PA7':'ENC_B','PE0':'HX_DOUT_MCU','PE1':'HX_SCK_MCU','PE2':'BTN_FORWARD','PE3':'BTN_BACK','PE4':'BTN_SOFTSTOP','PC7':'SD_DETECT','PC8':'SD_D0','PC9':'SD_D1','PC10':'SD_D2','PC11':'SD_D3','PC12':'SD_CLK_MCU','PD2':'SD_CMD','PA9':'UART_TX','PA10':'UART_RX'}
        for pin,net in connections.items():self.assertEqual(self.pin('U_MCU',MCU_NAMES.index(pin)+1),net)
    def test_all_mcu_power_pins(self):
        self.assertEqual(len(MCU_NAMES),100)
        for p in [11,19,28,50,75,100]:
            self.assertEqual(self.pin('U_MCU',p),'3V3');self.assertEqual(self.c['C_VDD'+str(p)]['part'],'100n')
        for p in [10,20,27,74,99]:self.assertEqual(self.pin('U_MCU',p),'GND')
        self.assertEqual(self.pin('U_MCU',6),'3V3')
        self.assertEqual([self.pin('U_MCU',p) for p in [21,22]],['VDDA','VDDA'])
    def test_vcap_only_has_capacitor_and_mcu(self):
        for n,pin in [(1,49),(2,73)]:
            net=f'VCAP{n}';attached=[(c['ref'],p) for c in self.c.values() for p,v in c['pins'].items() if v==net]
            self.assertCountEqual(attached,[('U_MCU',str(pin)),(f'C_VCAP{n}','1')])
            self.assertEqual(self.pin(f'C_VCAP{n}',2),'GND');self.assertEqual(self.c[f'C_VCAP{n}']['part'],'2u2')
    def test_power_protection_and_regulator_pinout(self):
        self.assertEqual(self.c['D_REVERSE']['pins'],{'1':'VIN_PROTECTED','2':'VIN_FUSED'})
        self.assertEqual([self.pin('U_BUCK',p) for p in [1,2,3,7,8,9]],['GND','VIN_PROTECTED','VIN_PROTECTED','BUCK_BOOT','BUCK_SW','GND'])
        self.assertEqual(self.c['C_BOOT']['pins'],{'1':'BUCK_BOOT','2':'BUCK_SW'})
        for r in ['C_IN','C_IN_HF','C_IN_BULK']:
            voltage=float(re.search(r'[0-9.]+',self.parts[self.c[r]['part']]['voltage']).group())
            self.assertGreaterEqual(voltage,63)
        self.assertEqual([self.pin('U_LDO',p) for p in [1,2,4,8,9]],['3V3','5V','GND','5V','GND'])
    def test_can_contract(self):
        self.assertEqual(self.c['U_CAN']['pins'],{'1':'CAN_TX','2':'GND','3':'5V','4':'CAN_RX','5':'3V3','6':'CAN_L','7':'CAN_H','8':'CAN_STB'})
        self.assertEqual(self.c['J_CAN']['pins'],{'1':'GND','2':'CAN_H','3':'CAN_L'})
        self.assertEqual(self.c['R_TERM']['part'],'120R');self.assertFalse(self.c['R_TERM']['dnp'])
        for rail in ['VCC','VIO']:
            self.assertEqual(self.c[f'C_CAN_{rail}_HF']['part'],'100n');self.assertEqual(self.c[f'C_CAN_{rail}']['part'],'1u')
    def test_storage_and_interfaces(self):
        expected=['SD_D2','SD_D3','SD_CMD','3V3','SD_CLK','GND','SD_D0','SD_D1','SD_DETECT']
        self.assertEqual([self.pin('J_TF',i) for i in range(1,10)],expected)
        for net in ['SD_CMD','SD_D0','SD_D1','SD_D2','SD_D3']:self.assertEqual(self.c['R_'+net]['part'],'47k')
        self.assertEqual(self.c['C_SD_BULK']['part'],'10u')
        self.assertEqual(self.c['J_HX711']['pins'],{'1':'3V3','2':'GND','3':'HX_DOUT','4':'HX_SCK'})
        self.assertEqual(self.c['J_SWD']['pins'],{'1':'3V3','2':'GND','3':'SWDIO','4':'SWCLK','5':'NRST'})
    def test_no_dangling_intent_nets(self):
        nets={}
        for c in self.c.values():
            for pin,net in c['pins'].items():
                if net:nets.setdefault(net,[]).append((c['ref'],pin))
        self.assertEqual({k:v for k,v in nets.items() if len(v)<2},{})
    def test_mechanical_and_package_policy(self):
        self.assertEqual(len(self.d['mountingHoles']),4)
        for h in self.d['mountingHoles']:
            self.assertEqual(h['diameter_mm'],3.2);self.assertFalse(h['plated']);self.assertIsNone(h['net']);self.assertIsNone(h['xy'])
        for p in self.parts.values():self.assertNotIn(p['package'],['0402','0603'])
        self.assertEqual(self.parts['led']['package'],'0805')
    def test_feedback_and_peak_current(self):
        vout=1+100000/24900
        self.assertGreater(vout,4.95);self.assertLess(vout,5.05)
        ripple=vout*(1-vout/25.2)/(400000*15e-6)
        self.assertLess(2+ripple/2,4.5)

if __name__=='__main__':unittest.main(verbosity=2)
