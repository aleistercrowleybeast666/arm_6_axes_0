"""Rev.A pin-level source of truth. No PCB creation or firmware edits."""
from pathlib import Path
import csv
import json

ROOT = Path(__file__).resolve().parents[1]
SHEETS = ['01_POWER', '02_MCU', '03_CAN', '04_STORAGE', '05_UI_IO', '06_DEBUG']
PART_CODES = {
    'mcu': 'C28730', 'buck': 'C5149193', 'ldo': 'C4943338', 'can': 'C5382542',
    'tf': 'C91145', 'xt30': 'C431092', 'ph2': 'C295747', 'ph3': 'C265101',
    'ph4': 'C265102', 'ph5': 'C265104', 'gh3': 'C514175', 'crystal': 'C5181482',
    'fuse': 'C3099', 'reverse': 'C2848687', 'input_tvs': 'C173526',
    'can_tvs': 'C134951', 'inductor': 'C40000', 'led': 'C205441', 'tp': 'C2906768',
    '100n': 'C1711', '100n100': 'C398816', '1u': 'C28323', '2u2': 'C87994',
    '10u': 'C15850', '22u': 'C309062', '4u7HV': 'C2325876', '47uHV': 'C178377',
    '18p': 'C1797', '0R': 'C17477', '10k': 'C17414', '100k': 'C149504',
    '64k9': 'C17788', '24k9': 'C17571', '4k7': 'C17673', '47k': 'C17713',
    '120R': 'C17437', '33R': 'C17634', '470R': 'C17710',
}

# ST DS8626 LQFP100 Figure 13, also cross-checked against CubeMX MCU XML.
MCU_NAMES = '''PE2 PE3 PE4 PE5 PE6 VBAT PC13 PC14 PC15 VSS VDD PH0 PH1 NRST
PC0 PC1 PC2 PC3 VDD VSSA VREF+ VDDA PA0 PA1 PA2 PA3 VSS VDD PA4 PA5 PA6 PA7
PC4 PC5 PB0 PB1 PB2 PE7 PE8 PE9 PE10 PE11 PE12 PE13 PE14 PE15 PB10 PB11
VCAP_1 VDD PB12 PB13 PB14 PB15 PD8 PD9 PD10 PD11 PD12 PD13 PD14 PD15 PC6
PC7 PC8 PC9 PA8 PA9 PA10 PA11 PA12 PA13 VCAP_2 VSS VDD PA14 PA15 PC10 PC11
PC12 PD0 PD1 PD2 PD3 PD4 PD5 PD6 PD7 PB3 PB4 PB5 PB6 PB7 BOOT0 PB8 PB9 PE0
PE1 VSS VDD'''.split()
MCU_SIGNALS = {
    'PE2':'BTN_FORWARD', 'PE3':'BTN_BACK', 'PE4':'BTN_SOFTSTOP',
    'PB8':'CAN_RX', 'PB9':'CAN_TX', 'PB6':'OLED_SCL', 'PB7':'OLED_SDA',
    'PA6':'ENC_A', 'PA7':'ENC_B', 'PE0':'HX_DOUT_MCU', 'PE1':'HX_SCK_MCU',
    'PC7':'SD_DETECT', 'PC8':'SD_D0', 'PC9':'SD_D1', 'PC10':'SD_D2',
    'PC11':'SD_D3', 'PC12':'SD_CLK_MCU', 'PD2':'SD_CMD',
    'PA9':'UART_TX', 'PA10':'UART_RX', 'PA13':'SWDIO', 'PA14':'SWCLK',
    'PH0':'HSE_IN', 'PH1':'HSE_OUT', 'PA11':'USB_DM_RESERVED', 'PA12':'USB_DP_RESERVED',
    'NRST':'NRST', 'BOOT0':'BOOT0', 'VBAT':'3V3', 'VDD':'3V3', 'VSS':'GND',
    'VDDA':'VDDA', 'VREF+':'VDDA', 'VSSA':'GND', 'VCAP_1':'VCAP1', 'VCAP_2':'VCAP2',
    'PB2':'BOOT1',
}

def build_design():
    components=[]
    def add(ref, part, sheet, pins, xy, notes='', names=None, dnp=False):
        components.append(dict(ref=ref,part=part,sheet=SHEETS[sheet-1],
            pins={str(k):v for k,v in pins.items()}, x=xy[0],y=xy[1],
            notes=notes,pinNames={str(k):v for k,v in (names or {}).items()},dnp=dnp))
    def pair(ref,part,sheet,a,b,x,y,notes=''):
        add(ref,part,sheet,{'1':a,'2':b},(x,y),notes)
    # Positions are schematic 10-mil units, never PCB coordinates.
    add('J_POWER','xt30',1,{1:'24V_IN',2:'GND'},(80,120),'PIN1 +24V_LOGIC; PIN2 GND. Logic branch only; verify moulded +/− against library pads.')
    pair('F1','fuse',1,'24V_IN','VIN_FUSED',230,120,'1A fast fuse; upstream battery branch fuse required; interrupt rating coordination pending.')
    # Diode pin 1 is cathode, pin 2 is anode; checked during library audit.
    add('D_REVERSE','reverse',1,{1:'VIN_PROTECTED',2:'VIN_FUSED'},(390,120),'100V/3A series Schottky; cathode downstream.',{1:'K',2:'A'})
    add('D_INPUT','input_tvs',1,{1:'VIN_PROTECTED',2:'GND'},(570,120),'SMBJ33A, VRWM33V, VC53.3V at rated pulse.',{1:'K',2:'A'})
    pair('C_IN_BULK','47uHV',1,'VIN_PROTECTED','GND',740,120,'47uF/63V polymer. Pin1 positive.')
    pair('C_IN','4u7HV',1,'VIN_PROTECTED','GND',910,120,'4.7uF/100V X7R; immediately across VIN/GND.')
    pair('C_IN_HF','100n100',1,'VIN_PROTECTED','GND',1080,120,'100nF/100V; minimum hot-loop area.')
    add('U_BUCK','buck',1,{1:'GND',2:'VIN_PROTECTED',3:'VIN_PROTECTED',4:'BUCK_RT',5:'BUCK_FB',6:'BUCK_PG',7:'BUCK_BOOT',8:'BUCK_SW',9:'GND'},(300,420),
        'CRITICAL LAYOUT AREA. EN=VIN; exposed pad GND. No external VCC pin.',{1:'GND',2:'EN',3:'VIN',4:'RT/SYNC',5:'FB',6:'PG',7:'BOOT',8:'SW',9:'EP'})
    pair('R_RT','64k9',1,'BUCK_RT','GND',70,420,'400kHz, TI Table 8-1.')
    pair('C_BOOT','100n',1,'BUCK_BOOT','BUCK_SW',520,360,'100nF directly BOOT to SW, not GND.')
    pair('L1','inductor',1,'BUCK_SW','5V',720,420,'15uH, 4.5A thermal/8A saturation at specified test conditions.')
    for n,x in enumerate([900,1060,1220],1):pair(f'C_OUT{n}','22u',1,'5V','GND',x,420,'22uF/25V X7R, 1210; verify DC-bias effective capacitance.')
    pair('R_FB_TOP','100k',1,'5V','BUCK_FB',540,550)
    pair('R_FB_BOTTOM','24k9',1,'BUCK_FB','GND',740,550,'Vout=1V*(1+100k/24.9k)=5.016V.')
    pair('R_PG','100k',1,'3V3','BUCK_PG',900,550,'Open drain PG pulled to 3V3; debug test point only.')
    add('U_LDO','ldo',1,{1:'3V3',2:'5V',3:None,4:'GND',5:None,6:None,7:None,8:'5V',9:'GND'},(300,800),
        'AP7361C SO-8EP (SP), EN=IN. Thermal design limits continuous current.',{1:'OUT',2:'EN',3:'NC',4:'GND',5:'NC',6:'NC',7:'NC',8:'IN',9:'EP'})
    pair('C_LDO_IN','10u',1,'5V','GND',70,800)
    pair('C_LDO_OUT','10u',1,'3V3','GND',550,800)
    pair('R_LED','470R',1,'3V3','LED_A',780,800,'About 1mA at Vf=2.83V; verify LED batch at low current.')
    add('LED_PWR','led',1,{1:'GND',2:'LED_A'},(1000,800),'BLUE 0805; silk PWR.',{1:'K',2:'A'})
    mcu_pins={str(i):MCU_SIGNALS.get(n) for i,n in enumerate(MCU_NAMES,1)}
    add('U_MCU','mcu',2,mcu_pins,(420,600),'All unused GPIO explicitly no-connect; no pin reassignment.',dict(enumerate(MCU_NAMES,1)))
    for i,pin in enumerate([11,19,28,50,75,100]):
        pair(f'C_VDD{pin}','100n',2,'3V3','GND',850+(i%3)*170,180+(i//3)*150,f'Place directly at U_MCU pin {pin}.')
    pair('C_MCU_BULK','10u',2,'3V3','GND',850,500)
    pair('C_VBAT','100n',2,'3V3','GND',1030,500,'VBAT tied to 3V3; no battery backup.')
    pair('C_VCAP1','2u2',2,'VCAP1','GND',850,680,'2.2uF low-ESR <=2ohm. NEVER connect VCAP to 3V3.')
    pair('C_VCAP2','2u2',2,'VCAP2','GND',1030,680,'2.2uF low-ESR <=2ohm. NEVER connect VCAP to 3V3.')
    pair('R_AVDD','0R',2,'3V3','VDDA',850,850,'0R feed; footprint can accept ferrite only after current/impedance review.')
    pair('C_AVDD_HF','100n',2,'VDDA','GND',1030,850)
    pair('C_AVDD_BULK','1u',2,'VDDA','GND',1200,850)
    pair('R_BOOT0','10k',2,'BOOT0','GND',120,1100)
    pair('R_BOOT1','10k',2,'BOOT1','GND',300,1100,'PB2 boot strap low; currently unused by firmware.')
    pair('R_RESET','10k',2,'3V3','NRST',480,1100)
    pair('C_RESET','100n',2,'NRST','GND',660,1100,'SWD reset access; no optional reset button fitted.')
    add('Y1','crystal',2,{1:'HSE_IN',2:'GND',3:'HSE_OUT',4:'GND'},(1000,1100),'8MHz CL12pF ESR80ohm max; 5032. Pads 2/4 case ground.')
    pair('C_HSE_IN','18p',2,'HSE_IN','GND',850,1260,'C1=C2=18pF with estimated Cstray=3pF gives CL=12pF; verify startup/drive.')
    pair('C_HSE_OUT','18p',2,'HSE_OUT','GND',1150,1260)
    add('U_CAN','can',3,{1:'CAN_TX',2:'GND',3:'5V',4:'CAN_RX',5:'3V3',6:'CAN_L',7:'CAN_H',8:'CAN_STB'},(360,330),
        'SIT1042AQT/3 SOP8; STB LOW normal/high-speed; CAN1 1Mbps.',{1:'TXD',2:'GND',3:'VCC',4:'RXD',5:'VIO',6:'CANL',7:'CANH',8:'STB'})
    pair('R_STB','0R',3,'CAN_STB','GND',110,500)
    for ref,key,net,x,y in [('C_CAN_VCC_HF','100n','5V',100,100),('C_CAN_VCC','1u','5V',280,100),('C_CAN_VIO_HF','100n','3V3',470,100),('C_CAN_VIO','1u','3V3',660,100)]:pair(ref,key,3,net,'GND',x,y)
    pair('R_TERM','120R',3,'CAN_H','CAN_L',680,300,'FIXED 120ohm. Main controller is a physical bus end. No switch/jumper.')
    pair('D_CAN_H','can_tvs',3,'CAN_H','GND',900,180,'Bidirectional SMAJ13CA; verify capacitive loading at 1Mbps.')
    pair('D_CAN_L','can_tvs',3,'CAN_L','GND',900,460,'Bidirectional SMAJ13CA; no substitution in Rev.A.')
    add('J_CAN','gh3',3,{1:'GND',2:'CAN_H',3:'CAN_L'},(1160,330),'JST GH 1.25mm; numbered pin order is harness contract.')
    add('J_TF','tf',4,{1:'SD_D2',2:'SD_D3',3:'SD_CMD',4:'3V3',5:'SD_CLK',6:'GND',7:'SD_D0',8:'SD_D1',9:'SD_DETECT'},(800,360),
        'HRO TF-01A: contact Cd=9, closes to grounded shell when inserted. All shell pins GND; library pad audit required.',
        {1:'DAT2',2:'DAT3',3:'CMD',4:'VDD',5:'CLK',6:'VSS',7:'DAT0',8:'DAT1',9:'CD'})
    components[-1]['extraGroundPinsByName']=['GND','SHELL','SHIELD','CASE']
    for i,n in enumerate(['SD_D0','SD_D1','SD_D2','SD_D3','SD_CMD']):pair('R_'+n,'47k',4,'3V3',n,100+i*190,100,'SD pull-up 47k in 10k..100k range; CLK has none.')
    pair('R_SD_CLK','33R',4,'SD_CLK_MCU','SD_CLK',180,360,'Place near MCU PC12; 4-bit SDIO currently 8MHz.')
    pair('R_SD_DETECT','10k',4,'3V3','SD_DETECT',350,520,'Active-low mechanical detect independent of DAT3.')
    pair('C_SD_HF','100n',4,'3V3','GND',650,630)
    pair('C_SD_BULK','10u',4,'3V3','GND',860,630)
    add('J_OLED','ph4',5,{1:'3V3',2:'GND',3:'OLED_SCL',4:'OLED_SDA'},(130,160),'SSD1306 1.3in 128x64, I2C address0x3C; short harness.')
    pair('R_SCL','4k7',5,'3V3','OLED_SCL',380,120,'Default fitted; DNP only if module already has suitable strong pull-ups.')
    pair('R_SDA','4k7',5,'3V3','OLED_SDA',570,120,'Default fitted; verify 400kHz rise time and total pull-up resistance.')
    add('J_ENCODER','ph3',5,{1:'GND',2:'ENC_A',3:'ENC_B'},(900,170),'Mechanical encoder A/B only; no push switch.')
    pair('R_ENC_A','10k',5,'3V3','ENC_A',1120,100)
    pair('R_ENC_B','10k',5,'3V3','ENC_B',1120,260)
    for i,(name,net) in enumerate([('FORWARD','BTN_FORWARD'),('BACK','BTN_BACK'),('SOFTSTOP','BTN_SOFTSTOP')]):
        x=180+i*410
        add('J_'+name,'ph2',5,{1:net,2:'GND'},(x,470),'Active LOW. SoftStop is NOT hardware E-STOP.')
        pair('R_'+name,'10k',5,'3V3',net,x,640,'Software debounce; no default RC capacitor.')
    add('J_HX711','ph4',5,{1:'3V3',2:'GND',3:'HX_DOUT',4:'HX_SCK'},(190,930),'Remote HX711 close to load cell. 3.3V only; DOUT must not be 5V.')
    pair('R_HX_DOUT','33R',5,'HX_DOUT','HX_DOUT_MCU',530,870)
    pair('R_HX_SCK','33R',5,'HX_SCK_MCU','HX_SCK',530,1030)
    pair('C_HX_LOCAL','100n',5,'3V3','GND',850,950,'Board-side connector decoupling; remote HX711 requires its own local capacitors.')
    add('J_UART','ph3',6,{1:'GND',2:'UART_TX',3:'UART_RX'},(130,170),'UART 3V3 TTL 115200; TX/RX named from controller viewpoint.')
    add('J_SWD','ph5',6,{1:'3V3',2:'GND',3:'SWDIO',4:'SWCLK',5:'NRST'},(600,170),'3V3 is target voltage reference, not external power input.')
    tps=['24V_IN','5V','3V3','GND','CAN_TX','CAN_RX','CAN_H','CAN_L','UART_TX','UART_RX','NRST','BOOT0','SD_CLK','SWDIO','SWCLK','USB_DM_RESERVED','USB_DP_RESERVED','BUCK_PG']
    for i,net in enumerate(tps):add('TP_'+net,'tp',6,{1:net},(100+(i%6)*205,460+(i//6)*190),'Top-access SMT test point; keep high-speed stubs short.')
    return dict(name='arm_6_axes_0_Prototype0_RevA',revision='A',sheets=SHEETS,components=components,
        mountingHoles=[dict(ref=f'MH{i}',type='MOUNT_HOLE_M3',diameter_mm=3.2,plated=False,net=None,keepout_diameter_mm=6,xy=None) for i in range(1,5)],
        notes={
            '01_POWER':['LOGIC SUPPLY ONLY | 6S LiPo 22.2V nominal / 25.2V full','CRITICAL LAYOUT AREA: TI LMR38020 400kHz / 15uH / 5.016V','No motor power distribution. Input protection requires upstream branch fuse.'],
            '02_MCU':['STM32F407VET6 | HSE 8MHz | SYSCLK 168MHz','6 x VDD100nF; VCAP1/2 each 2.2uF to GND ONLY','Pin map follows arm_6_axes_0.ioc; unused GPIO no-connect.'],
            '03_CAN':['Main controller contains fixed 120 ohm termination.','SIT1042AQT/3: VCC5V / VIO3V3; each 100nF + 1uF','SMAJ13CA remains as requested; signal integrity validation pending.'],
            '04_STORAGE':['microSD 4-bit SDIO | external 47k pull-ups on CMD and DAT0..3','CLK33R near MCU; no CLK pull-up | PC7 card detect active LOW'],
            '05_UI_IO':['All external logic 3.3V. Follow frozen connector pin numbers.','SoftStop is software-controlled stop. Hardware E-STOP is external.'],
            '06_DEBUG':['UART 3V3 TTL | SWD target reference 3V3','4 x MOUNT_HOLE_M3, 3.2mm NPTH, no net/copper, 6mm keepout','PCB NOT STARTED. Mounting-hole X/Y frozen next revision stage.'],
        })

def generate():
    design=build_design()
    catalog=json.loads((ROOT/'design/parts.json').read_text(encoding='utf-8'))
    design['parts']=catalog
    (ROOT/'design/controller_revA.json').write_text(json.dumps(design,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    fields='RefDes Qty Category Value Manufacturer MPN LCSC_PN Package Footprint Voltage_Rating Tolerance Power_Rating Assembly DNP Notes Datasheet'.split()
    with (ROOT/'bom/arm_6_axes_0_revA_bom.csv').open('w',newline='',encoding='utf-8-sig') as f:
        writer=csv.DictWriter(f,fieldnames=fields);writer.writeheader()
        for c in design['components']:
            p=catalog[c['part']]
            writer.writerow(dict(RefDes=c['ref'],Qty=1,Category=p['category'],Value=p['value'],Manufacturer=p['manufacturer'],MPN=p['mpn'],LCSC_PN=p['lcsc'],Package=p['package'],Footprint=p.get('footprint','OFFICIAL_LIBRARY_PENDING_AUDIT'),Voltage_Rating=p.get('voltage',''),Tolerance=p.get('tolerance',''),Power_Rating=p.get('power',''),Assembly='THT_TOP' if c['part']=='xt30' else 'SMT_TOP',DNP='YES' if c['dnp'] else 'NO',Notes=c['notes'],Datasheet=p['datasheet']))
        for h in design['mountingHoles']:
            writer.writerow(dict(RefDes=h['ref'],Qty=1,Category='Mechanical feature',Value='M3 3.2mm NPTH',Manufacturer='PCB fabrication',MPN='MOUNT_HOLE_M3',LCSC_PN='N/A',Package='3.2mm NPTH',Footprint='Mechanical definition; no PCB generated',Assembly='FABRICATION',DNP='N/A',Notes='Unplated; no net; 6mm mechanical keepout; X/Y pending layout.'))
    print(f'Generated {len(design["components"])} components, {len(catalog)} purchasing lines, 6 sheets, 4 mechanical holes.')

if __name__=='__main__':generate()
