import sys, os

def generate():
    print("Regenerating Circuit Schematic with Dual Magnetometers & Electrical Corrections...")
    
    svg_parts = []
    
    # 1. Header & Defs
    svg_parts.append('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 3800 2800" width="3800" height="2800">
<defs>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&amp;family=JetBrains+Mono:wght@500;700;800&amp;display=swap');
    text { font-family: 'Inter', -apple-system, sans-serif; }
    .mono { font-family: 'JetBrains Mono', monospace; }
    .title { font-weight: 800; fill: #FFFFFF; font-size: 30px; letter-spacing: -0.5px; }
    .subtitle { font-weight: 500; fill: #94A3B8; font-size: 14px; }
    .mod-title { font-weight: 800; fill: #F8FAFC; font-size: 18px; }
    .mod-sub { font-weight: 600; fill: #94A3B8; font-size: 11px; }
    .wire { fill: none; stroke-linecap: round; stroke-linejoin: round; }
    .wire-glow { fill: none; stroke-linecap: round; stroke-linejoin: round; opacity: 0.15; }
    .chip-label { font-weight: 800; fill: #E2E8F0; font-family: 'JetBrains Mono', monospace; }
  </style>

  <filter id="cardShadow" x="-5%" y="-5%" width="110%" height="110%">
    <feDropShadow dx="0" dy="12" stdDeviation="16" flood-color="#000000" flood-opacity="0.6"/>
  </filter>

  <filter id="boardShadow" x="-6%" y="-3%" width="112%" height="106%">
    <feDropShadow dx="0" dy="20" stdDeviation="24" flood-color="#000000" flood-opacity="0.8"/>
  </filter>

  <filter id="glow" x="-20%" y="-20%" width="140%" height="140%">
    <feGaussianBlur stdDeviation="4" result="blur"/>
    <feComposite in="SourceGraphic" in2="blur" operator="over"/>
  </filter>

  <linearGradient id="bgGrad" x1="0" y1="0" x2="1" y2="1">
    <stop offset="0%" stop-color="#070A12"/>
    <stop offset="100%" stop-color="#0E1526"/>
  </linearGradient>

  <linearGradient id="boardGrad" x1="0" y1="0" x2="0" y2="1">
    <stop offset="0%" stop-color="#162032"/>
    <stop offset="50%" stop-color="#0F172A"/>
    <stop offset="100%" stop-color="#0A0F1D"/>
  </linearGradient>

  <linearGradient id="mcuGrad" x1="0" y1="0" x2="1" y2="1">
    <stop offset="0%" stop-color="#222C3D"/>
    <stop offset="100%" stop-color="#121824"/>
  </linearGradient>

  <linearGradient id="goldPad" x1="0" y1="0" x2="1" y2="1">
    <stop offset="0%" stop-color="#FCD34D"/>
    <stop offset="100%" stop-color="#D97706"/>
  </linearGradient>

  <pattern id="dotGrid" width="30" height="30" patternUnits="userSpaceOnUse">
    <circle cx="15" cy="15" r="1.2" fill="#1E293B" opacity="0.45"/>
  </pattern>
</defs>

<!-- Background -->
<rect width="3800" height="2800" fill="url(#bgGrad)"/>
<rect width="3800" height="2800" fill="url(#dotGrid)"/>
''')

    # 2. Header Title Card with Dual Mag Callout
    svg_parts.append('''
<!-- Header Title Card -->
<g transform="translate(80, 30)">
  <rect width="3640" height="155" rx="16" fill="#101726" stroke="#1E293B" stroke-width="1.8" filter="url(#cardShadow)"/>
  <text x="40" y="44" class="title">DevEBox STM32H750VBT6 Universal Flight &amp; Rover Controller (Dual-Magnetometer Revision)</text>
  <text x="40" y="74" class="subtitle">Complete Circuit &amp; Schematic Interconnect • PX4 Autopilot / NuttX Architecture • DUAL COMPASS (Mast Mag 1 on I2C1 + Frame Mag 2 on I2C2) • 8 Motor Outputs • Isolated Power Domains</text>
  
  <!-- Badges / Legend -->
  <g transform="translate(40, 98)">
    <g transform="translate(0, 0)"><rect width="68" height="24" rx="4" fill="#EF4444" fill-opacity="0.15" stroke="#EF4444" stroke-width="1.2"/><text x="34" y="16" fill="#EF4444" font-size="11" font-weight="700" text-anchor="middle">3.3V PWR</text></g>
    <g transform="translate(80, 0)"><rect width="60" height="24" rx="4" fill="#DC2626" fill-opacity="0.15" stroke="#DC2626" stroke-width="1.2"/><text x="30" y="16" fill="#DC2626" font-size="11" font-weight="700" text-anchor="middle">5V PWR</text></g>
    <g transform="translate(152, 0)"><rect width="52" height="24" rx="4" fill="#475569" fill-opacity="0.2" stroke="#64748B" stroke-width="1.2"/><text x="26" y="16" fill="#CBD5E1" font-size="11" font-weight="700" text-anchor="middle">GND</text></g>
    <g transform="translate(216, 0)"><rect width="76" height="24" rx="4" fill="#3B82F6" fill-opacity="0.15" stroke="#3B82F6" stroke-width="1.2"/><text x="38" y="16" fill="#60A5FA" font-size="11" font-weight="700" text-anchor="middle">SPI BUS</text></g>
    <g transform="translate(304, 0)"><rect width="84" height="24" rx="4" fill="#8B5CF6" fill-opacity="0.15" stroke="#8B5CF6" stroke-width="1.2"/><text x="42" y="16" fill="#A78BFA" font-size="11" font-weight="700" text-anchor="middle">I2C1 (MAST)</text></g>
    <g transform="translate(400, 0)"><rect width="88" height="24" rx="4" fill="#A855F7" fill-opacity="0.15" stroke="#A855F7" stroke-width="1.2"/><text x="44" y="16" fill="#C084FC" font-size="11" font-weight="700" text-anchor="middle">I2C2 (FRAME)</text></g>
    <g transform="translate(500, 0)"><rect width="84" height="24" rx="4" fill="#06B6D4" fill-opacity="0.15" stroke="#06B6D4" stroke-width="1.2"/><text x="42" y="16" fill="#22D3EE" font-size="11" font-weight="700" text-anchor="middle">UART/SERIAL</text></g>
    <g transform="translate(596, 0)"><rect width="88" height="24" rx="4" fill="#F97316" fill-opacity="0.15" stroke="#F97316" stroke-width="1.2"/><text x="44" y="16" fill="#FB923C" font-size="11" font-weight="700" text-anchor="middle">PWM MOTORS</text></g>
    <g transform="translate(696, 0)"><rect width="84" height="24" rx="4" fill="#10B981" fill-opacity="0.15" stroke="#10B981" stroke-width="1.2"/><text x="42" y="16" fill="#34D399" font-size="11" font-weight="700" text-anchor="middle">ADC ANALOG</text></g>
    <g transform="translate(792, 0)"><rect width="80" height="24" rx="4" fill="#F43F5E" fill-opacity="0.15" stroke="#F43F5E" stroke-width="1.2"/><text x="40" y="16" fill="#FB7185" font-size="11" font-weight="700" text-anchor="middle">EXTI INT</text></g>
    <g transform="translate(884, 0)"><rect width="88" height="24" rx="4" fill="#EC4899" fill-opacity="0.15" stroke="#EC4899" stroke-width="1.2"/><text x="44" y="16" fill="#F472B6" font-size="11" font-weight="700" text-anchor="middle">QSPI FLASH</text></g>
    <g transform="translate(984, 0)"><rect width="92" height="24" rx="4" fill="#6366F1" fill-opacity="0.15" stroke="#6366F1" stroke-width="1.2"/><text x="46" y="16" fill="#818CF8" font-size="11" font-weight="700" text-anchor="middle">USB NATIVE</text></g>
  </g>
</g>
''')

    # 3. Center Board Geometry
    bx = 1620
    by = 240
    bw = 560
    bh = 2420
    
    h_start_y = 340
    h_step = 48
    
    h1_pins = {}
    h2_pins = {}
    for i in range(1, 45):
        y = h_start_y + (i - 1) * h_step
        h1_pins[i] = (bx + 28, y)
        h2_pins[i] = (bx + bw - 28, y)

    svg_parts.append(f'''
<!-- ==================== CENTRAL DEVEBOX STM32H750VBT6 BOARD ==================== -->
<g id="DevEBoxBoard">
  <rect x="{bx}" y="{by}" width="{bw}" height="{bh}" rx="28" fill="url(#boardGrad)" stroke="#334155" stroke-width="2.5" filter="url(#boardShadow)"/>
  
  <circle cx="{bx + 35}" cy="{by + 35}" r="16" fill="#0B0F19" stroke="url(#goldPad)" stroke-width="4"/>
  <circle cx="{bx + bw - 35}" cy="{by + 35}" r="16" fill="#0B0F19" stroke="url(#goldPad)" stroke-width="4"/>
  <circle cx="{bx + 35}" cy="{by + bh - 35}" r="16" fill="#0B0F19" stroke="url(#goldPad)" stroke-width="4"/>
  <circle cx="{bx + bw - 35}" cy="{by + bh - 35}" r="16" fill="#0B0F19" stroke="url(#goldPad)" stroke-width="4"/>

  <text x="{bx + bw/2}" y="{by + 50}" fill="#94A3B8" font-size="16" font-weight="800" text-anchor="middle" letter-spacing="2">DEVEBOX STM32H7XX-M</text>
  <text x="{bx + bw/2}" y="{by + 72}" fill="#64748B" font-size="12" font-weight="600" text-anchor="middle">CORE BOARD (STM32H750VBT6 / H743VIT6)</text>

  <!-- J4 Header (TFT / SPI2) -->
  <g transform="translate({bx + bw/2 - 130}, {by + 100})">
    <rect width="260" height="70" rx="6" fill="#1E293B" stroke="#475569" stroke-width="1.5"/>
    <text x="130" y="24" fill="#E2E8F0" font-size="12" font-weight="700" text-anchor="middle">J4 SPI2 / TFT HEADER (8-PIN)</text>
    <text x="130" y="42" fill="#94A3B8" font-size="10" font-family="'JetBrains Mono', monospace" text-anchor="middle">3V3 GND PB15 PB13 PB12 PB14 PB1 PB0</text>
    <text x="130" y="58" fill="#3B82F6" font-size="9" font-weight="700" text-anchor="middle">SPARE SPI2 FOR OPTICAL FLOW / SPI LIDAR</text>
  </g>

  <!-- J1 SWD Header (5-pin) -->
  <g transform="translate({bx + bw/2 - 90}, {by + 190})">
    <rect width="180" height="42" rx="4" fill="#1E293B" stroke="#475569" stroke-width="1.2"/>
    <text x="90" y="18" fill="#E2E8F0" font-size="10" font-weight="700" text-anchor="middle">J1 SWD DEBUG</text>
    <text x="90" y="32" fill="#94A3B8" font-size="9" font-family="'JetBrains Mono', monospace" text-anchor="middle">BT0 3V3 GND DIO CLK</text>
  </g>

  <!-- W25Q64JV External QSPI Flash Chip -->
  <g transform="translate({bx + bw/2 - 75}, {by + 260})">
    <rect width="150" height="90" rx="8" fill="#111827" stroke="#EC4899" stroke-width="1.8" filter="url(#cardShadow)"/>
    <circle cx="20" cy="20" r="4" fill="#EC4899"/>
    <text x="75" y="38" fill="#F472B6" font-size="13" font-weight="800" text-anchor="middle">WINBOND</text>
    <text x="75" y="56" fill="#FFFFFF" font-size="14" font-weight="800" font-family="'JetBrains Mono', monospace" text-anchor="middle">W25Q64JV</text>
    <text x="75" y="74" fill="#94A3B8" font-size="10" font-weight="600" text-anchor="middle">8MB QSPI (0x90000000)</text>
  </g>

  <!-- HSE 25MHz Crystal -->
  <g transform="translate({bx + bw/2 - 60}, {by + 380})">
    <rect width="120" height="40" rx="6" fill="#334155" stroke="#94A3B8" stroke-width="1.2"/>
    <text x="60" y="24" fill="#F8FAFC" font-size="11" font-weight="700" font-family="'JetBrains Mono', monospace" text-anchor="middle">HSE 25.000 MHz</text>
  </g>

  <!-- STM32H750VBT6 MCU Chip -->
  <g transform="translate({bx + bw/2 - 140}, {by + 800})">
    <rect width="280" height="280" rx="16" fill="url(#mcuGrad)" stroke="#475569" stroke-width="2.5" filter="url(#mcuShadow)"/>
    <circle cx="30" cy="30" r="7" fill="#64748B"/>
    <text x="140" y="85" fill="#3B82F6" font-size="24" font-weight="800" text-anchor="middle">ST</text>
    <text x="140" y="125" class="chip-label" font-size="17" text-anchor="middle">STM32H750</text>
    <text x="140" y="152" class="chip-label" font-size="17" text-anchor="middle">VBT6</text>
    <text x="140" y="188" fill="#94A3B8" font-size="12" font-weight="600" text-anchor="middle">ARM Cortex-M7</text>
    <text x="140" y="208" fill="#38BDF8" font-size="12" font-weight="700" text-anchor="middle">480 MHz • 1MB RAM</text>
    <text x="140" y="234" fill="#64748B" font-size="10" text-anchor="middle">100-PIN LQFP</text>

    <!-- Visual IC pin leads -->
    ''' + ''.join([f'<rect x="-14" y="{30 + k*10}" width="14" height="4" fill="#94A3B8"/>' for k in range(22)]) + '''
    ''' + ''.join([f'<rect x="280" y="{30 + k*10}" width="14" height="4" fill="#94A3B8"/>' for k in range(22)]) + '''
    ''' + ''.join([f'<rect x="{30 + k*10}" y="-14" width="4" height="14" fill="#94A3B8"/>' for k in range(22)]) + '''
    ''' + ''.join([f'<rect x="{30 + k*10}" y="280" width="4" height="14" fill="#94A3B8"/>' for k in range(22)]) + '''
  </g>

  <!-- MicroSD Card Slot (SDMMC1) -->
  <g transform="translate({bx + bw/2 - 80}, {by + 1400})">
    <rect width="160" height="220" rx="8" fill="#1E293B" stroke="#64748B" stroke-width="1.8"/>
    <text x="80" y="40" fill="#E2E8F0" font-size="13" font-weight="700" text-anchor="middle">MicroSD (TF)</text>
    <text x="80" y="60" fill="#94A3B8" font-size="10" font-weight="500" text-anchor="middle">PX4 FLIGHT LOGGING</text>
    <path d="M 30,120 L 130,120 L 130,210 L 30,210 Z" fill="#0B0F19" stroke="#334155"/>
  </g>

  <!-- LEDs & Buttons -->
  <g transform="translate({bx + bw/2 - 160}, {by + 1900})">
    <circle cx="40" cy="30" r="8" fill="#10B981" filter="url(#glow)"/>
    <text x="40" y="55" fill="#A7F3D0" font-size="10" font-weight="700" text-anchor="middle">PWR D1</text>
    <circle cx="120" cy="30" r="8" fill="#3B82F6" filter="url(#glow)"/>
    <text x="120" y="55" fill="#93C5FD" font-size="10" font-weight="700" text-anchor="middle">LED D2 (PA1)</text>
    <rect x="180" y="16" width="36" height="28" rx="4" fill="#475569" stroke="#94A3B8"/>
    <text x="198" y="55" fill="#CBD5E1" font-size="10" font-weight="700" text-anchor="middle">RESET</text>
    <rect x="240" y="16" width="36" height="28" rx="4" fill="#475569" stroke="#94A3B8"/>
    <text x="258" y="55" fill="#CBD5E1" font-size="10" font-weight="700" text-anchor="middle">K1 (PE3)</text>
  </g>

  <!-- Onboard Micro-USB / USB-C Receptacle -->
  <g transform="translate({bx + bw/2 - 80}, {by + bh - 70})">
    <rect width="160" height="70" rx="10" fill="#334155" stroke="#94A3B8" stroke-width="2"/>
    <rect x="25" y="18" width="110" height="34" rx="14" fill="#0B0F19" stroke="#64748B" stroke-width="1.5"/>
    <rect x="45" y="30" width="70" height="10" rx="5" fill="url(#goldPad)"/>
    <text x="80" y="-12" fill="#818CF8" font-size="13" font-weight="800" text-anchor="middle">USB NATIVE (PA11/12)</text>
  </g>
''')

    # 4. Header 1 & 2 Pin Tables (Incorporating I2C2 Frame Sensors & 5V Clarifications)
    h1_labels = {
        1: ("3V3", "#EF4444", "3.3V Power Bus"),
        2: ("3V3", "#EF4444", "3.3V Power Bus"),
        3: ("GND", "#475569", "Ground Plane"),
        4: ("GND", "#475569", "Ground Plane"),
        5: ("VDDA", "#EF4444", "Analog 3.3V"),
        6: ("VREF+", "#EF4444", "ADC Reference"),
        11: ("PB11", "#A855F7", "I2C2_SDA (Frame Mag & Baro)"),
        12: ("PB10", "#A855F7", "I2C2_SCL (Frame Mag & Baro)"),
        14: ("PE14", "#F97316", "MOTOR 4 (TIM1_CH4)"),
        15: ("PE13", "#F97316", "MOTOR 3 (TIM1_CH3)"),
        17: ("PE11", "#F97316", "MOTOR 2 (TIM1_CH2)"),
        19: ("PE9", "#F97316", "MOTOR 1 (TIM1_CH1)"),
        20: ("PE8", "#06B6D4", "UART7_TX (RC Telem)"),
        21: ("PE7", "#06B6D4", "UART7_RX (RC Data)"),
        23: ("PB1", "#F97316", "MOTOR 8 (TIM3_CH4)"),
        24: ("PB0", "#F97316", "MOTOR 7 (TIM3_CH3)"),
        27: ("PA7", "#3B82F6", "BMI160 MOSI (SPI1)"),
        28: ("PA6", "#3B82F6", "BMI160 MISO (SPI1)"),
        29: ("PA5", "#3B82F6", "BMI160 SCK (SPI1)"),
        30: ("PA4", "#3B82F6", "BMI160 CS (SPI1)"),
        33: ("PA1", "#3B82F6", "User LED D2"),
        34: ("PA0", "#10B981", "BATT VOLTAGE (11:1 ADC1)"),
        38: ("PC0", "#10B981", "BATT CURRENT (3.3V ADC1)"),
        42: ("PE4", "#F43F5E", "BMI160 INT1 (EXTI4 1kHz)"),
        43: ("PE3", "#475569", "User Button K1"),
        22: ("PB2", "#EC4899", "QSPI_CLK"),
        44: ("PE2", "#EC4899", "QSPI_IO2"),
    }

    h2_labels = {
        1: ("5V", "#DC2626", "5V Power Bus (Isolated)"),
        2: ("5V", "#DC2626", "5V Power Bus (Isolated)"),
        3: ("3V3", "#EF4444", "3.3V Power Bus"),
        4: ("3V3", "#EF4444", "3.3V Power Bus"),
        5: ("GND", "#475569", "Ground Plane"),
        6: ("GND", "#475569", "Ground Plane"),
        7: ("PD9", "#06B6D4", "GPS_TX (USART3_RX)"),
        8: ("PD8", "#06B6D4", "GPS_RX (USART3_TX)"),
        9: ("PD11", "#EC4899", "QSPI_IO0"),
        11: ("PD13", "#EC4899", "QSPI_IO3"),
        12: ("PD12", "#EC4899", "QSPI_IO1"),
        15: ("PC7", "#F97316", "MOTOR 6 (TIM3_CH2)"),
        16: ("PC6", "#F97316", "MOTOR 5 (TIM3_CH1)"),
        19: ("PA9", "#06B6D4", "USART1_TX (Debug Console)"),
        21: ("PA11", "#6366F1", "USB_DM (Native)"),
        22: ("PA10", "#06B6D4", "USART1_RX (Debug Console)"),
        24: ("PA12", "#6366F1", "USB_DP (Native)"),
        27: ("PD0", "#06B6D4", "ESP32_TX (UART4_RX)"),
        30: ("PD1", "#06B6D4", "ESP32_RX (UART4_TX)"),
        33: ("PD6", "#06B6D4", "RPI_TX (USART2_RX @ 921k)"),
        34: ("PD5", "#06B6D4", "RPI_RX (USART2_TX @ 921k)"),
        40: ("PB6", "#EC4899", "QSPI_CS"),
        41: ("PB9", "#8B5CF6", "MAST_MAG_SDA (I2C1)"),
        42: ("PB8", "#8B5CF6", "MAST_MAG_SCL (I2C1)"),
    }

    # Render Header 1 pins
    for pin_num in range(1, 45):
        cx, cy = h1_pins[pin_num]
        info = h1_labels.get(pin_num, (f"P{pin_num}", "#334155", ""))
        p_name, p_color, p_desc = info
        is_active = pin_num in h1_labels
        svg_parts.append(f'''
  <circle cx="{cx}" cy="{cy}" r="9" fill="#0B0F19" stroke="{p_color if is_active else '#334155'}" stroke-width="2"/>
  <circle cx="{cx + 24}" cy="{cy}" r="9" fill="#0B0F19" stroke="#334155" stroke-width="1.5"/>
  <circle cx="{cx}" cy="{cy}" r="3.5" fill="{p_color if is_active else '#475569'}"/>
  <circle cx="{cx + 24}" cy="{cy}" r="3.5" fill="#475569"/>
  <text x="{cx + 50}" y="{cy + 4}" fill="{ '#FFFFFF' if is_active else '#64748B'}" font-size="11" font-weight="700" class="mono">{pin_num:02d}</text>
  <text x="{cx + 74}" y="{cy + 4}" fill="{p_color if is_active else '#475569'}" font-size="11" font-weight="700" class="mono">{p_name}</text>
''')

    # Render Header 2 pins
    for pin_num in range(1, 45):
        cx, cy = h2_pins[pin_num]
        info = h2_labels.get(pin_num, (f"P{pin_num}", "#334155", ""))
        p_name, p_color, p_desc = info
        is_active = pin_num in h2_labels
        svg_parts.append(f'''
  <circle cx="{cx}" cy="{cy}" r="9" fill="#0B0F19" stroke="{p_color if is_active else '#334155'}" stroke-width="2"/>
  <circle cx="{cx - 24}" cy="{cy}" r="9" fill="#0B0F19" stroke="#334155" stroke-width="1.5"/>
  <circle cx="{cx}" cy="{cy}" r="3.5" fill="{p_color if is_active else '#475569'}"/>
  <circle cx="{cx - 24}" cy="{cy}" r="3.5" fill="#475569"/>
  <text x="{cx - 50}" y="{cy + 4}" fill="{ '#FFFFFF' if is_active else '#64748B'}" font-size="11" font-weight="700" class="mono" text-anchor="end">{pin_num:02d}</text>
  <text x="{cx - 74}" y="{cy + 4}" fill="{p_color if is_active else '#475569'}" font-size="11" font-weight="700" class="mono" text-anchor="end">{p_name}</text>
''')

    svg_parts.append('</g><!-- End DevEBoxBoard -->\n')

    # 5. External Modules
    def draw_module(mx, my, mw, mh, title, subtitle, color, pins):
        res = []
        res.append(f'''
<g transform="translate({mx}, {my})" filter="url(#cardShadow)">
  <rect width="{mw}" height="{mh}" rx="14" fill="#101726" stroke="{color}" stroke-width="2"/>
  <rect x="0" y="0" width="{mw}" height="52" rx="14" fill="{color}" fill-opacity="0.12"/>
  <line x1="0" y1="52" x2="{mw}" y2="52" stroke="{color}" stroke-width="1.5" stroke-opacity="0.3"/>
  <circle cx="26" cy="26" r="6" fill="{color}"/>
  <text x="44" y="32" class="mod-title">{title}</text>
  <text x="{mw - 16}" y="32" class="mod-sub" text-anchor="end">{subtitle}</text>
''')
        pin_coords = {}
        for idx, (p_label, p_col, side) in enumerate(pins):
            py = 80 + idx * 36
            if side == "right":
                px = mw - 14
                res.append(f'''
  <circle cx="{px}" cy="{py}" r="6" fill="#0B0F19" stroke="{p_col}" stroke-width="2"/>
  <circle cx="{px}" cy="{py}" r="2.5" fill="{p_col}"/>
  <text x="{px - 14}" y="{py + 4}" fill="#E2E8F0" class="mono" font-size="12" font-weight="700" text-anchor="end">{p_label}</text>
''')
                pin_coords[p_label] = (mx + px, my + py)
            else:
                px = 14
                res.append(f'''
  <circle cx="{px}" cy="{py}" r="6" fill="#0B0F19" stroke="{p_col}" stroke-width="2"/>
  <circle cx="{px}" cy="{py}" r="2.5" fill="{p_col}"/>
  <text x="{px + 14}" y="{py + 4}" fill="#E2E8F0" class="mono" font-size="12" font-weight="700" text-anchor="start">{p_label}</text>
''')
                pin_coords[p_label] = (mx + px, my + py)
        res.append('</g>\n')
        return ''.join(res), pin_coords

    # Module 1: BMI160 SPI IMU (Top Left)
    m1_pins = [
        ("VIN", "#EF4444", "right"),
        ("GND", "#475569", "right"),
        ("CS", "#3B82F6", "right"),
        ("SCL (SCK)", "#3B82F6", "right"),
        ("SDA (MOSI)", "#3B82F6", "right"),
        ("SAO (MISO)", "#3B82F6", "right"),
        ("INT1 (DRDY)", "#F43F5E", "right"),
    ]
    card_bmi, coords_bmi = draw_module(80, 240, 480, 350, "BMI160 6-AXIS IMU", "PRIMARY FLIGHT SENSOR (SPI1)", "#3B82F6", m1_pins)
    svg_parts.append(card_bmi)

    # Module 2: Local Frame Magnetometer GY-271 & BMP280 Baro (On I2C2) (Upper Middle Left)
    m2_pins = [
        ("VCC (3.3V)", "#EF4444", "right"),
        ("GND", "#475569", "right"),
        ("I2C2 SCL", "#A855F7", "right"),
        ("I2C2 SDA", "#A855F7", "right"),
    ]
    card_frame_i2c, coords_frame_i2c = draw_module(80, 630, 480, 240, "GY-271 MAG 2 + BMP280", "FRAME BUS (I2C2: PB10/11)", "#A855F7", m2_pins)
    svg_parts.append(card_frame_i2c)

    # Module 3: ELRS / CRSF Receiver (Middle Left)
    m3_pins = [
        ("5V (BEC)", "#DC2626", "right"),
        ("GND", "#475569", "right"),
        ("TX (Telemetry)", "#06B6D4", "right"),
        ("RX (RC Channels)", "#06B6D4", "right"),
    ]
    card_rc, coords_rc = draw_module(80, 910, 480, 240, "ELRS / CRSF RECEIVER", "RADIO LINK (UART7 @ 420k)", "#06B6D4", m3_pins)
    svg_parts.append(card_rc)

    # Module 4: Battery & Power Monitor (Lower Left)
    m4_pins = [
        ("BATT (+)", "#DC2626", "right"),
        ("VOLT (11:1 DIV)", "#10B981", "right"),
        ("CURR (3.3V MAX)", "#10B981", "right"),
        ("GND (-)", "#475569", "right"),
    ]
    card_pwr, coords_pwr = draw_module(80, 1190, 480, 250, "BATTERY MONITOR", "11:1 DIVIDER + 3.3V CURRENT SHUNT", "#10B981", m4_pins)
    svg_parts.append(card_pwr)

    # Module 5: Actuator Bank Motors 1-4 (TIM1) (Bottom Left)
    m5_pins = [
        ("MOTOR 1 (PE9)", "#F97316", "right"),
        ("MOTOR 2 (PE11)", "#F97316", "right"),
        ("MOTOR 3 (PE13)", "#F97316", "right"),
        ("MOTOR 4 (PE14)", "#F97316", "right"),
        ("10k PULL-DOWNS", "#64748B", "right"),
        ("GND RAIL", "#475569", "right"),
    ]
    card_m1_4, coords_m1_4 = draw_module(80, 1480, 480, 310, "ACTUATORS: MOTORS 1-4", "TIM1 PWM / DSHOT (QUAD / HEXA)", "#F97316", m5_pins)
    svg_parts.append(card_m1_4)

    # Module 6: GPS & Mast Magnetometer (Mag 1) (Top Right)
    m6_pins = [
        ("5V (BEC)", "#DC2626", "left"),
        ("GND", "#475569", "left"),
        ("GPS TX", "#06B6D4", "left"),
        ("GPS RX", "#06B6D4", "left"),
        ("MAG 1 SCL", "#8B5CF6", "left"),
        ("MAG 1 SDA", "#8B5CF6", "left"),
    ]
    card_gps, coords_gps = draw_module(3240, 240, 480, 310, "GPS &amp; MAST COMPASS (MAG 1)", "PRIMARY NAV (UART3 + I2C1: PB8/9)", "#8B5CF6", m6_pins)
    svg_parts.append(card_gps)

    # Module 7: ESP32 Telemetry (Middle Right)
    m7_pins = [
        ("5V (EXT BEC)", "#DC2626", "left"),
        ("GND", "#475569", "left"),
        ("TXD0", "#06B6D4", "left"),
        ("RXD0", "#06B6D4", "left"),
    ]
    card_esp, coords_esp = draw_module(3240, 590, 480, 240, "ESP32 TELEMETRY", "WIFI / BLE MAVLINK (UART4)", "#06B6D4", m7_pins)
    svg_parts.append(card_esp)

    # Module 8: Raspberry Pi Companion Computer (Lower Right)
    m8_pins = [
        ("GPIO 14 (TX)", "#06B6D4", "left"),
        ("GPIO 15 (RX)", "#06B6D4", "left"),
        ("GND (Pin 6)", "#475569", "left"),
        ("5V 3A+ (EXT BEC)", "#DC2626", "left"),
    ]
    card_rpi, coords_rpi = draw_module(3240, 870, 480, 240, "RASPBERRY PI 4 / 5", "ROS2 / uXRCE-DDS (USART2 @ 921k)", "#10B981", m8_pins)
    svg_parts.append(card_rpi)

    # Module 9: Actuators Motors 5-8 (TIM3) (Bottom Right)
    m9_pins = [
        ("MOTOR 5 (PC6)", "#F97316", "left"),
        ("MOTOR 6 (PC7)", "#F97316", "left"),
        ("MOTOR 7 (PB0)", "#F97316", "left"),
        ("MOTOR 8 (PB1)", "#F97316", "left"),
        ("10k PULL-DOWNS", "#64748B", "left"),
        ("GND RAIL", "#475569", "left"),
    ]
    card_m5_8, coords_m5_8 = draw_module(3240, 1150, 480, 310, "ACTUATORS: MOTORS 5-8", "TIM3 PWM (HEXA / 8-WHEEL ROVER)", "#F97316", m9_pins)
    svg_parts.append(card_m5_8)

    # Module 10: Power Distribution & Diode-OR Board (Bottom Center-Right)
    svg_parts.append('''
<g transform="translate(3240, 1500)" filter="url(#cardShadow)">
  <rect width="480" height="280" rx="14" fill="#101726" stroke="#DC2626" stroke-width="2"/>
  <rect x="0" y="0" width="480" height="52" rx="14" fill="#DC2626" fill-opacity="0.12"/>
  <circle cx="26" cy="26" r="6" fill="#DC2626"/>
  <text x="44" y="32" class="mod-title">POWER &amp; DIODE-OR ISOLATION</text>
  <text x="464" y="32" class="mod-sub" text-anchor="end">PROTECT LAPTOP &amp; BEC</text>
  
  <text x="24" y="85" fill="#E2E8F0" font-size="12" font-weight="700">• 5V BEC Diode-OR:</text>
  <text x="40" y="105" fill="#94A3B8" font-size="11">Place Schottky Diode (SS14/BAT54) on 5V BEC</text>
  <text x="40" y="123" fill="#EF4444" font-size="11" font-weight="700">Prevents back-feeding 5V into laptop USB!</text>

  <text x="24" y="155" fill="#E2E8F0" font-size="12" font-weight="700">• Dedicated BEC Rails (Common GND Only):</text>
  <text x="40" y="175" fill="#94A3B8" font-size="11">BEC 1: 5V 3A–5A dedicated for Raspberry Pi</text>
  <text x="40" y="193" fill="#94A3B8" font-size="11">BEC 2: 5V 1A for GPS, ESP32, and ELRS Receiver</text>
  <text x="40" y="211" fill="#94A3B8" font-size="11">Onboard AMS1117-3.3: Powers DevEBox MCU only!</text>
</g>
''')

    # 6. Orthogonal Wiring Buses
    wires = []
    def route_wire(p1, p2, color, stroke_w=2.2):
        x1, y1 = p1
        x2, y2 = p2
        mid_x = (x1 + x2) / 2
        path = f"M {x1},{y1} C {mid_x},{y1} {mid_x},{y2} {x2},{y2}"
        wires.append(f'''
  <path d="{path}" class="wire-glow" stroke="{color}" stroke-width="{stroke_w + 5}"/>
  <path d="{path}" class="wire" stroke="{color}" stroke-width="{stroke_w}"/>
  <circle cx="{x1}" cy="{y1}" r="3" fill="{color}"/>
  <circle cx="{x2}" cy="{y2}" r="3" fill="{color}"/>
''')

    # Wire routes
    # BMI160 SPI1 -> Header 1
    route_wire(coords_bmi["CS"], h1_pins[30], "#3B82F6")
    route_wire(coords_bmi["SCL (SCK)"], h1_pins[29], "#3B82F6")
    route_wire(coords_bmi["SAO (MISO)"], h1_pins[28], "#3B82F6")
    route_wire(coords_bmi["SDA (MOSI)"], h1_pins[27], "#3B82F6")
    route_wire(coords_bmi["INT1 (DRDY)"], h1_pins[42], "#F43F5E")
    route_wire(coords_bmi["VIN"], h1_pins[1], "#EF4444")
    route_wire(coords_bmi["GND"], h1_pins[3], "#475569")

    # Local Frame Sensors (GY-271 Mag 2 & BMP280 Baro) -> Header 1 (I2C2: PB10/PB11)
    route_wire(coords_frame_i2c["I2C2 SCL"], h1_pins[12], "#A855F7")
    route_wire(coords_frame_i2c["I2C2 SDA"], h1_pins[11], "#A855F7")
    route_wire(coords_frame_i2c["VCC (3.3V)"], h1_pins[2], "#EF4444")
    route_wire(coords_frame_i2c["GND"], h1_pins[4], "#475569")

    # RC Receiver UART7 -> Header 1
    route_wire(coords_rc["TX (Telemetry)"], h1_pins[20], "#06B6D4")
    route_wire(coords_rc["RX (RC Channels)"], h1_pins[21], "#06B6D4")

    # Battery Monitor ADC1 -> Header 1
    route_wire(coords_pwr["VOLT (11:1 DIV)"], h1_pins[34], "#10B981")
    route_wire(coords_pwr["CURR (3.3V MAX)"], h1_pins[38], "#10B981")

    # Motors 1-4 TIM1 -> Header 1
    route_wire(coords_m1_4["MOTOR 1 (PE9)"], h1_pins[19], "#F97316")
    route_wire(coords_m1_4["MOTOR 2 (PE11)"], h1_pins[17], "#F97316")
    route_wire(coords_m1_4["MOTOR 3 (PE13)"], h1_pins[15], "#F97316")
    route_wire(coords_m1_4["MOTOR 4 (PE14)"], h1_pins[14], "#F97316")

    # GPS & Mast Compass (Mag 1) -> Header 2 (USART3 + I2C1: PB8/PB9)
    route_wire(coords_gps["GPS TX"], h2_pins[7], "#06B6D4")
    route_wire(coords_gps["GPS RX"], h2_pins[8], "#06B6D4")
    route_wire(coords_gps["MAG 1 SCL"], h2_pins[42], "#8B5CF6")
    route_wire(coords_gps["MAG 1 SDA"], h2_pins[41], "#8B5CF6")
    route_wire(coords_gps["5V (BEC)"], h2_pins[1], "#DC2626")
    route_wire(coords_gps["GND"], h2_pins[5], "#475569")

    # ESP32 UART4 -> Header 2
    route_wire(coords_esp["TXD0"], h2_pins[27], "#06B6D4")
    route_wire(coords_esp["RXD0"], h2_pins[30], "#06B6D4")

    # Raspberry Pi USART2 -> Header 2
    route_wire(coords_rpi["GPIO 14 (TX)"], h2_pins[33], "#06B6D4")
    route_wire(coords_rpi["GPIO 15 (RX)"], h2_pins[34], "#06B6D4")

    # Motors 5-8 TIM3 -> Header 2 & Header 1
    route_wire(coords_m5_8["MOTOR 5 (PC6)"], h2_pins[16], "#F97316")
    route_wire(coords_m5_8["MOTOR 6 (PC7)"], h2_pins[15], "#F97316")
    route_wire(coords_m5_8["MOTOR 7 (PB0)"], h1_pins[24], "#F97316")
    route_wire(coords_m5_8["MOTOR 8 (PB1)"], h1_pins[23], "#F97316")

    svg_parts.append('<g id="WiringBuses">\n' + ''.join(wires) + '</g>\n')

    # 7. Bottom Bench Station Callout
    svg_parts.append(f'''
<g transform="translate({bx + bw/2 - 250}, {by + bh + 40})" filter="url(#cardShadow)">
  <rect width="500" height="70" rx="12" fill="#101726" stroke="#6366F1" stroke-width="2"/>
  <text x="250" y="30" fill="#FFFFFF" font-size="16" font-weight="800" text-anchor="middle">PC / LAPTOP GROUND STATION (QGroundControl)</text>
  <text x="250" y="52" fill="#818CF8" font-size="12" font-weight="600" text-anchor="middle">Native USB Virtual COM Port • DFU Firmware Upload (Bench Only)</text>
  <line x1="250" y1="0" x2="250" y2="-20" stroke="#6366F1" stroke-width="3" stroke-dasharray="4 4"/>
</g>
''')

    svg_parts.append('</svg>')
    
    with open('Hardware/schematics/flight_rover_schematic.svg', 'w') as f:
        f.write(''.join(svg_parts))
    print("Regenerated Hardware/schematics/flight_rover_schematic.svg successfully!")

generate()
