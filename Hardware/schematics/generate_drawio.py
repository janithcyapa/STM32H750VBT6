import os
import xml.etree.ElementTree as ET
import xml.dom.minidom as minidom

def create_drawio_xml():
    # 24 columns A to X, 18 rows 01 to 18
    cols = [chr(c) for c in range(ord('A'), ord('X')+1)] # 24 columns
    pitch = 44 # 44px standard grid pitch (simulating 2.54mm hole-to-hole spacing)
    grid_x0 = 1320
    grid_y0 = 850
    # A1 is at bottom-left:
    # Column A is index 0 -> X = grid_x0 + 0 * pitch
    # Row 1 is bottom -> Y = grid_y0 + (18 - 1) * pitch
    
    # We build the mxfile XML structure with LIGHT THEME
    mxfile = ET.Element('mxfile', host="app.diagrams.net", type="device")
    diagram = ET.SubElement(mxfile, 'diagram', id="dotboard_perfboard_light", name="DevEBox DotBoard Circuit Prototyping (Light)")
    model = ET.SubElement(diagram, 'mxGraphModel', dx="2400", dy="1800", grid="1", gridSize="10", guides="1", tooltips="1", connect="1", arrows="1", fold="1", page="1", pageScale="1", pageWidth="3800", pageHeight="2800", background="#FFFFFF")
    root = ET.SubElement(model, 'root')

    ET.SubElement(root, 'mxCell', id="0")
    ET.SubElement(root, 'mxCell', id="1", parent="0")

    elem_id = 100
    def next_id():
        nonlocal elem_id
        elem_id += 1
        return str(elem_id)

    # 1. Title Banner at top (Light Theme)
    title_id = next_id()
    title_cell = ET.SubElement(root, 'mxCell', id=title_id, parent="1", vertex="1",
                               value="<div style='font-size:24px; font-weight:800; color:#0F172A;'>DevEBox STM32H750VBT6 Dot-Board (Perfboard) Hardware Prototype Grid</div><div style='font-size:14px; font-weight:600; color:#475569; margin-top:4px;'>Grid: Columns A–X (24) &bull; Rows 01–18 &bull; A,1 at Bottom-Left &bull; Sensor Header Pins Spaced at Exact 44px Grid Pitch &bull; Bank 1 (B2–W3), Bank 2 (B16–W17), TFT J4 (V6–V13)</div>",
                               style="rounded=1;whiteSpace=wrap;html=1;fillColor=#F8FAFC;strokeColor=#CBD5E1;strokeWidth=1.5;align=left;spacingLeft=30;verticalAlign=middle;")
    t_geom = ET.SubElement(title_cell, 'mxGeometry', x="200", y="60", width="3400", height="90")
    t_geom.set('as', 'geometry')

    # Legend / Color Key (Light Theme)
    legend_id = next_id()
    legend_html = ("<div style='font-size:12px; color:#1E293B; font-family:monospace;'>"
                   "<span style='background:#EF4444; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>3.3V PWR</span> &nbsp;"
                   "<span style='background:#DC2626; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>5V PWR</span> &nbsp;"
                   "<span style='background:#334155; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>GND</span> &nbsp;"
                   "<span style='background:#2563EB; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>SPI (BMI160)</span> &nbsp;"
                   "<span style='background:#7C3AED; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>I2C1 (Mast Mag 1)</span> &nbsp;"
                   "<span style='background:#9333EA; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>I2C2 (Frame Mag 2+Baro)</span> &nbsp;"
                   "<span style='background:#0891B2; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>UART (GPS/Pi/ESP/RC)</span> &nbsp;"
                   "<span style='background:#EA580C; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>PWM (Motors 1-8)</span> &nbsp;"
                   "<span style='background:#059669; color:#fff; padding:4px 10px; border-radius:4px; font-weight:bold;'>ADC (Battery/Current)</span>"
                   "</div>")
    leg_cell = ET.SubElement(root, 'mxCell', id=legend_id, parent="1", vertex="1",
                             value=legend_html,
                             style="rounded=1;whiteSpace=wrap;html=1;fillColor=#F8FAFC;strokeColor=#CBD5E1;strokeWidth=1;align=center;verticalAlign=middle;")
    leg_geom = ET.SubElement(leg_cell, 'mxGeometry', x="200", y="165", width="3400", height="45")
    leg_geom.set('as', 'geometry')

    # 2. Perfboard Backplane (Authentic FR4 / Phenolic Board Tan)
    b_width = 24 * pitch + 130
    b_height = 18 * pitch + 130
    board_bg_id = next_id()
    board_bg = ET.SubElement(root, 'mxCell', id=board_bg_id, parent="1", vertex="1",
                             value="",
                             style="rounded=1;whiteSpace=wrap;html=1;fillColor=#FAF4DF;strokeColor=#C2945D;strokeWidth=3;arcSize=3;")
    bg_geom = ET.SubElement(board_bg, 'mxGeometry', x=str(grid_x0 - 65), y=str(grid_y0 - 65), width=str(b_width), height=str(b_height))
    bg_geom.set('as', 'geometry')

    # Silk outline inside board
    silk_id = next_id()
    silk = ET.SubElement(root, 'mxCell', id=silk_id, parent="1", vertex="1",
                         value="<div style='color:#A47137; font-weight:800; font-size:15px; letter-spacing:2px; text-align:center;'>DEV-EBOX STM32H750VBT6 PROTO-BOARD BENCH (A–X &bull; 01–18)</div>",
                         style="rounded=1;whiteSpace=wrap;html=1;fillColor=none;strokeColor=#C2945D;strokeWidth=1.5;dashed=1;verticalAlign=top;spacingTop=10;")
    silk_geom = ET.SubElement(silk, 'mxGeometry', x=str(grid_x0 - 45), y=str(grid_y0 - 45), width=str(b_width - 40), height=str(b_height - 40))
    silk_geom.set('as', 'geometry')

    # Helper coordinate calculator
    def get_hole_pos(col_char, row_num):
        col_idx = cols.index(col_char)
        x = grid_x0 + col_idx * pitch
        y = grid_y0 + (18 - row_num) * pitch
        return x, y

    # Dictionary of hole cell IDs for connecting wires later
    hole_cells = {}

    # Define Bank 1 mapping (Header 1: Rows 2 and 3, Cols B to W)
    bank1_data = {
        'W': (('3V3', '3.3V Rail', '#EF4444'), ('3V3', '3.3V Rail', '#EF4444')),
        'V': (('GND', 'Ground', '#334155'), ('GND', 'Ground', '#334155')),
        'U': (('VDDA', 'Analog VCC', '#D97706'), ('VREF+', 'ADC Ref', '#D97706')),
        'T': (('PB15', 'SPI2_MOSI', '#2563EB'), ('PB14', 'SPI2_MISO', '#2563EB')),
        'S': (('PB13', 'SPI2_SCK', '#2563EB'), ('PB12', 'SPI2_NSS', '#2563EB')),
        'R': (('PB11', 'I2C2_SDA (Mag2+Baro)', '#9333EA'), ('PB10', 'I2C2_SCL (Mag2+Baro)', '#9333EA')),
        'Q': (('PE15', 'GPIO', '#64748B'), ('PE14', 'TIM1_CH4 (Motor 4)', '#EA580C')),
        'P': (('PE13', 'TIM1_CH3 (Motor 3)', '#EA580C'), ('PE12', 'GPIO', '#64748B')),
        'O': (('PE11', 'TIM1_CH2 (Motor 2)', '#EA580C'), ('PE10', 'GPIO', '#64748B')),
        'N': (('PE9', 'TIM1_CH1 (Motor 1)', '#EA580C'), ('PE8', 'UART7_TX (RC Telem)', '#0891B2')),
        'M': (('PE7', 'UART7_RX (RC Data)', '#0891B2'), ('PB2', 'QSPI_CLK', '#DB2777')),
        'L': (('PB1', 'TIM3_CH4 (Motor 8)', '#EA580C'), ('PB0', 'TIM3_CH3 (Motor 7)', '#EA580C')),
        'K': (('PC5', 'K2 Button', '#64748B'), ('PC4', 'GPIO', '#64748B')),
        'J': (('PA7', 'SPI1_MOSI (BMI160)', '#2563EB'), ('PA6', 'SPI1_MISO (BMI160)', '#2563EB')),
        'I': (('PA5', 'SPI1_SCK (BMI160)', '#2563EB'), ('PA4', 'SPI1_NSS (BMI160 CS)', '#2563EB')),
        'H': (('PA3', 'GPIO', '#64748B'), ('PA2', 'GPIO', '#64748B')),
        'G': (('PA1', 'D2 Blue LED', '#0284C7'), ('PA0', 'ADC1_INP16 (Volt Sense)', '#059669')),
        'F': (('PC3', 'GPIO', '#64748B'), ('PC2', 'GPIO', '#64748B')),
        'E': (('PC1', 'GPIO', '#64748B'), ('PC0', 'ADC1_INP10 (Curr Sense)', '#059669')),
        'D': (('PC13', 'GPIO', '#64748B'), ('PE6', 'GPIO', '#64748B')),
        'C': (('PE5', 'GPIO', '#64748B'), ('PE4', 'EXTI4 (BMI160 INT1)', '#E11D48')),
        'B': (('PE3', 'K1 Button', '#64748B'), ('PE2', 'QSPI_IO2', '#DB2777'))
    }

    # Define Bank 2 mapping (Header 2: Rows 17 and 16, Cols B to W)
    bank2_data = {
        'W': (('5V', '5V Rail', '#DC2626'), ('5V', '5V Rail', '#DC2626')),
        'V': (('3V3', '3.3V Rail', '#EF4444'), ('3V3', '3.3V Rail', '#EF4444')),
        'U': (('GND', 'Ground', '#334155'), ('GND', 'Ground', '#334155')),
        'T': (('PD9', 'USART3_RX (GPS TX)', '#0891B2'), ('PD8', 'USART3_TX (GPS RX)', '#0891B2')),
        'S': (('PD11', 'QSPI_IO0', '#DB2777'), ('PD10', 'GPIO', '#64748B')),
        'R': (('PD13', 'QSPI_IO3', '#DB2777'), ('PD12', 'QSPI_IO1', '#DB2777')),
        'Q': (('PD15', 'GPIO', '#64748B'), ('PD14', 'GPIO', '#64748B')),
        'P': (('PC7', 'TIM3_CH2 (Motor 6)', '#EA580C'), ('PC6', 'TIM3_CH1 (Motor 5)', '#EA580C')),
        'O': (('PC9', 'SD_D1', '#64748B'), ('PC8', 'SD_D0', '#64748B')),
        'N': (('PA9', 'USART1_TX', '#0891B2'), ('PA8', 'GPIO', '#64748B')),
        'M': (('PA11', 'USB_DM', '#4F46E5'), ('PA10', 'USART1_RX', '#0891B2')),
        'L': (('PA15', 'GPIO', '#64748B'), ('PA12', 'USB_DP', '#4F46E5')),
        'K': (('PC11', 'SD_D3', '#64748B'), ('PC10', 'SD_D2', '#64748B')),
        'J': (('PD0', 'UART4_RX (ESP32 TX)', '#0891B2'), ('PC12', 'SD_CLK', '#64748B')),
        'I': (('PD2', 'SD_CMD', '#64748B'), ('PD1', 'UART4_TX (ESP32 RX)', '#0891B2')),
        'H': (('PD4', 'GPIO', '#64748B'), ('PD3', 'GPIO', '#64748B')),
        'G': (('PD6', 'USART2_RX (RPi TX)', '#0891B2'), ('PD5', 'USART2_TX (RPi RX)', '#0891B2')),
        'F': (('PB3', 'SWO Debug', '#64748B'), ('PD7', 'GPIO', '#64748B')),
        'E': (('PB5', 'GPIO', '#64748B'), ('PB4', 'GPIO', '#64748B')),
        'D': (('PB7', 'GPIO', '#64748B'), ('PB6', 'QSPI_CS', '#DB2777')),
        'C': (('PB9', 'I2C1_SDA (Mast Mag1)', '#7C3AED'), ('PB8', 'I2C1_SCL (Mast Mag1)', '#7C3AED')),
        'B': (('PE1', 'GPIO', '#64748B'), ('PE0', 'GPIO', '#64748B'))
    }

    # Define TFT Header J4: Column V, Rows 6 to 13
    tft_data = {
        6: ('3V3', '3.3V Power', '#EF4444'),
        7: ('GND', 'Ground', '#334155'),
        8: ('PB15', 'SPI2_MOSI (SDI)', '#2563EB'),
        9: ('PB13', 'SPI2_SCK (SCL)', '#2563EB'),
        10: ('PB12', 'SPI2_NSS (CS)', '#2563EB'),
        11: ('PB14', 'SPI2_MISO (SDO)', '#2563EB'),
        12: ('PB1', 'GPIO (D/C Select)', '#D97706'),
        13: ('PB0', 'GPIO (BLK Backlight)', '#D97706')
    }

    # Draw Column Headers (Top and Bottom)
    for c_idx, c_name in enumerate(cols):
        cx = grid_x0 + c_idx * pitch
        # Top label (above row 18)
        top_lbl = ET.SubElement(root, 'mxCell', id=next_id(), parent="1", vertex="1",
                                value=f"<b style='color:#0F172A; font-size:13px;'>{c_name}</b>",
                                style="text;html=1;align=center;verticalAlign=middle;fillColor=none;strokeColor=none;")
        t_g = ET.SubElement(top_lbl, 'mxGeometry', x=str(cx - 10), y=str(grid_y0 - 45), width="20", height="20")
        t_g.set('as', 'geometry')

        # Bottom label (below row 1)
        bot_lbl = ET.SubElement(root, 'mxCell', id=next_id(), parent="1", vertex="1",
                                value=f"<b style='color:#0F172A; font-size:13px;'>{c_name}</b>",
                                style="text;html=1;align=center;verticalAlign=middle;fillColor=none;strokeColor=none;")
        b_g = ET.SubElement(bot_lbl, 'mxGeometry', x=str(cx - 10), y=str(grid_y0 + 18 * pitch + 15), width="20", height="20")
        b_g.set('as', 'geometry')

    # Draw Row Headers (Left and Right)
    for r in range(1, 19):
        ry = grid_y0 + (18 - r) * pitch
        r_str = f"{r:02d}"
        # Left label (left of col A)
        left_lbl = ET.SubElement(root, 'mxCell', id=next_id(), parent="1", vertex="1",
                                 value=f"<b style='color:#0F172A; font-size:12px;'>{r_str}</b>",
                                 style="text;html=1;align=center;verticalAlign=middle;fillColor=none;strokeColor=none;")
        l_g = ET.SubElement(left_lbl, 'mxGeometry', x=str(grid_x0 - 45), y=str(ry - 10), width="25", height="20")
        l_g.set('as', 'geometry')

        # Right label (right of col X)
        right_lbl = ET.SubElement(root, 'mxCell', id=next_id(), parent="1", vertex="1",
                                  value=f"<b style='color:#0F172A; font-size:12px;'>{r_str}</b>",
                                  style="text;html=1;align=center;verticalAlign=middle;fillColor=none;strokeColor=none;")
        r_g = ET.SubElement(right_lbl, 'mxGeometry', x=str(grid_x0 + 24 * pitch + 15), y=str(ry - 10), width="25", height="20")
        r_g.set('as', 'geometry')

    # Draw All 24 x 18 = 432 Dot Holes on the Perfboard
    for c_idx, c_name in enumerate(cols):
        for r in range(1, 19):
            hx, hy = get_hole_pos(c_name, r)
            coord_str = f"{c_name}{r}"
            
            is_bank1 = (c_name in bank1_data and r in [2, 3])
            is_bank2 = (c_name in bank2_data and r in [16, 17])
            is_tft = (c_name == 'V' and r in tft_data)
            
            hole_val = ""
            hole_style = "ellipse;whiteSpace=wrap;html=1;aspect=fixed;fillColor=#FAF4DF;strokeColor=#D4A373;strokeWidth=1.5;"
            
            if is_bank1:
                sig_info = bank1_data[c_name][0 if r == 3 else 1]
                fill_c = sig_info[2]
                hole_val = f"<div style='font-size:7px; font-weight:800; color:#FFFFFF;'>{sig_info[0]}</div>"
                hole_style = f"ellipse;whiteSpace=wrap;html=1;aspect=fixed;fillColor={fill_c};strokeColor=#FFFFFF;strokeWidth=2;align=center;verticalAlign=middle;"
            elif is_bank2:
                sig_info = bank2_data[c_name][0 if r == 17 else 1]
                fill_c = sig_info[2]
                hole_val = f"<div style='font-size:7px; font-weight:800; color:#FFFFFF;'>{sig_info[0]}</div>"
                hole_style = f"ellipse;whiteSpace=wrap;html=1;aspect=fixed;fillColor={fill_c};strokeColor=#FFFFFF;strokeWidth=2;align=center;verticalAlign=middle;"
            elif is_tft:
                sig_info = tft_data[r]
                fill_c = sig_info[2]
                hole_val = f"<div style='font-size:7px; font-weight:800; color:#FFFFFF;'>{sig_info[0]}</div>"
                hole_style = f"ellipse;whiteSpace=wrap;html=1;aspect=fixed;fillColor={fill_c};strokeColor=#FFFFFF;strokeWidth=2;align=center;verticalAlign=middle;"
            
            hole_id = next_id()
            h_cell = ET.SubElement(root, 'mxCell', id=hole_id, parent="1", vertex="1",
                                   value=hole_val, style=hole_style)
            # Size of hole: 24x24 px centered
            h_geom = ET.SubElement(h_cell, 'mxGeometry', x=str(hx - 12), y=str(hy - 12), width="24", height="24")
            h_geom.set('as', 'geometry')
            
            hole_cells[coord_str] = hole_id

    # 3. Outer Pin Label Cards for Bank 1 (Below Row 1, Light Theme)
    b1_card_y = grid_y0 + 18 * pitch + 50
    for c_idx, c_name in enumerate(cols):
        if c_name in bank1_data:
            cx = grid_x0 + c_idx * pitch
            pin_top = bank1_data[c_name][0] # Row 3
            pin_bot = bank1_data[c_name][1] # Row 2
            
            card_id = next_id()
            card_html = (f"<div style='font-size:10px; font-family:monospace; line-height:1.2; text-align:center;'>"
                         f"<b style='color:#0F172A;'>Col {c_name}</b><br/>"
                         f"<span style='color:{pin_top[2]}; font-weight:bold;'>{c_name}3: {pin_top[0]}</span><br/>"
                         f"<span style='color:{pin_bot[2]}; font-weight:bold;'>{c_name}2: {pin_bot[0]}</span>"
                         f"</div>")
            card_cell = ET.SubElement(root, 'mxCell', id=card_id, parent="1", vertex="1",
                                      value=card_html,
                                      style="rounded=1;whiteSpace=wrap;html=1;fillColor=#FFFFFF;strokeColor=#CBD5E1;strokeWidth=1;align=center;")
            cg = ET.SubElement(card_cell, 'mxGeometry', x=str(cx - 20), y=str(b1_card_y), width="40", height="55")
            cg.set('as', 'geometry')

    # Outer Pin Label Cards for Bank 2 (Above Row 18, Light Theme)
    b2_card_y = grid_y0 - 130
    for c_idx, c_name in enumerate(cols):
        if c_name in bank2_data:
            cx = grid_x0 + c_idx * pitch
            pin_top = bank2_data[c_name][0] # Row 17
            pin_bot = bank2_data[c_name][1] # Row 16
            
            card_id = next_id()
            card_html = (f"<div style='font-size:10px; font-family:monospace; line-height:1.2; text-align:center;'>"
                         f"<b style='color:#0F172A;'>Col {c_name}</b><br/>"
                         f"<span style='color:{pin_top[2]}; font-weight:bold;'>{c_name}17: {pin_top[0]}</span><br/>"
                         f"<span style='color:{pin_bot[2]}; font-weight:bold;'>{c_name}16: {pin_bot[0]}</span>"
                         f"</div>")
            card_cell = ET.SubElement(root, 'mxCell', id=card_id, parent="1", vertex="1",
                                      value=card_html,
                                      style="rounded=1;whiteSpace=wrap;html=1;fillColor=#FFFFFF;strokeColor=#CBD5E1;strokeWidth=1;align=center;")
            cg = ET.SubElement(card_cell, 'mxGeometry', x=str(cx - 20), y=str(b2_card_y), width="40", height="55")
            cg.set('as', 'geometry')

    # TFT J4 Callout Label (to the right of col V, rows 6 to 13)
    tft_callout_id = next_id()
    tft_html = ("<div style='font-size:11px; font-family:monospace; line-height:1.4; color:#1E293B;'>"
                "<b style='color:#0284C7; font-size:12px;'>J4 TFT / SPI2 HEADER (V6–V13)</b><br/>"
                "<span style='color:#EF4444; font-weight:bold;'>V6: 3V3 Rail</span><br/>"
                "<span style='color:#334155; font-weight:bold;'>V7: GND Plane</span><br/>"
                "<span style='color:#2563EB; font-weight:bold;'>V8: PB15 (SDI/MOSI)</span><br/>"
                "<span style='color:#2563EB; font-weight:bold;'>V9: PB13 (SCL/SCK)</span><br/>"
                "<span style='color:#2563EB; font-weight:bold;'>V10: PB12 (CS/NSS)</span><br/>"
                "<span style='color:#2563EB; font-weight:bold;'>V11: PB14 (SDO/MISO)</span><br/>"
                "<span style='color:#D97706; font-weight:bold;'>V12: PB1 (D/C Select)</span><br/>"
                "<span style='color:#D97706; font-weight:bold;'>V13: PB0 (BLK Light)</span>"
                "</div>")
    tft_cell = ET.SubElement(root, 'mxCell', id=tft_callout_id, parent="1", vertex="1",
                             value=tft_html,
                             style="rounded=1;whiteSpace=wrap;html=1;fillColor=#FFFFFF;strokeColor=#0284C7;strokeWidth=1.5;align=left;spacingLeft=12;")
    tft_pos_x, tft_pos_y = get_hole_pos('X', 10)
    tg = ET.SubElement(tft_cell, 'mxGeometry', x=str(tft_pos_x + 50), y=str(tft_pos_y - 80), width="220", height="160")
    tg.set('as', 'geometry')

    # 4. SENSORS & MODULES AS REALISTIC SIP BREAKOUTS
    # Requirement: "the sensors need to have the same distance as the dot post in its connection since they are also headers"
    # Every breakout module has pin pads spaced at EXACTLY pitch (44px)!
    component_pins = {} # (comp_name, pin_name) -> cell_id

    def draw_sip_breakout(x, y, title, subtitle, pcb_color, pins):
        """
        Draws a breakout PCB with a 1xN SIP header whose pins are spaced at EXACTLY pitch (44px).
        pins: list of (pin_name, pin_color)
        """
        n_pins = len(pins)
        # Height is sized to fit all pins spaced at pitch
        h = (n_pins - 1) * pitch + 70
        w = 230
        
        # Module PCB Body
        comp_id = next_id()
        header_html = (f"<div style='font-size:12px; font-weight:800; color:#FFFFFF;'>{title}</div>"
                       f"<div style='font-size:10px; font-weight:600; color:#E2E8F0; margin-top:2px;'>{subtitle}</div>"
                       f"<div style='font-size:9px; color:#CBD5E1; margin-top:2px;'>[ 1x{n_pins} Header &bull; 44px Pitch ]</div>")
        c_cell = ET.SubElement(root, 'mxCell', id=comp_id, parent="1", vertex="1",
                               value=header_html,
                               style=f"rounded=1;whiteSpace=wrap;html=1;fillColor={pcb_color};strokeColor=#334155;strokeWidth=2;align=center;verticalAlign=top;spacingTop=10;")
        cg = ET.SubElement(c_cell, 'mxGeometry', x=str(x), y=str(y), width=str(w), height=str(h))
        cg.set('as', 'geometry')

        # Header background strip on right edge
        strip_id = next_id()
        strip_cell = ET.SubElement(root, 'mxCell', id=strip_id, parent="1", vertex="1",
                                   value="",
                                   style="rounded=1;whiteSpace=wrap;html=1;fillColor=#1E293B;strokeColor=#475569;strokeWidth=1;")
        strip_h = (n_pins - 1) * pitch + 36
        sg = ET.SubElement(strip_cell, 'mxGeometry', x=str(x + w - 40), y=str(y + 35), width="32", height=str(strip_h))
        sg.set('as', 'geometry')

        # Draw pin pads at EXACT pitch = 44px
        start_pin_y = y + 50
        for idx, (p_name, p_col) in enumerate(pins):
            py = start_pin_y + idx * pitch
            px = x + w - 24 # Pad X on the header strip
            
            # Pin Label
            lbl_id = next_id()
            lbl_cell = ET.SubElement(root, 'mxCell', id=lbl_id, parent="1", vertex="1",
                                     value=f"<b style='color:#FFFFFF; font-size:11px; font-family:monospace;'>{p_name}</b>",
                                     style="text;html=1;align=right;verticalAlign=middle;fillColor=none;strokeColor=none;")
            lg = ET.SubElement(lbl_cell, 'mxGeometry', x=str(x + 10), y=str(py - 10), width=str(w - 60), height="20")
            lg.set('as', 'geometry')

            # Pin Doughnut Hole (Matches the 24x24 pad size on the dot-board!)
            dot_id = next_id()
            # Pin 1 gets a square pad, other pins get round pads (standard PCB convention)
            pad_shape = "ellipse" if idx > 0 else "rectangle"
            dot_cell = ET.SubElement(root, 'mxCell', id=dot_id, parent="1", vertex="1",
                                     value=f"<b style='color:#FFFFFF; font-size:8px;'>{idx+1}</b>",
                                     style=f"{pad_shape};whiteSpace=wrap;html=1;aspect=fixed;fillColor={p_col};strokeColor=#FFFFFF;strokeWidth=2;align=center;verticalAlign=middle;")
            dg = ET.SubElement(dot_cell, 'mxGeometry', x=str(px - 12), y=str(py - 12), width="24", height="24")
            dg.set('as', 'geometry')

            component_pins[(title, p_name)] = dot_id
        return comp_id

    # Module 1: BMI160 6-Axis IMU Breakout (7-Pin Header, pitch=44px)
    bmi_pins = [
        ("VIN", "#EF4444"),
        ("GND", "#334155"),
        ("SCL (SCK)", "#2563EB"),
        ("SDA (MOSI)", "#2563EB"),
        ("SAO (MISO)", "#2563EB"),
        ("CS", "#2563EB"),
        ("INT1", "#E11D48"),
    ]
    draw_sip_breakout(200, 1420, "BMI160 6-AXIS IMU", "SPI1 PRIMARY (1 kHz EKF2)", "#1E3A8A", bmi_pins)

    # Module 2: GY-271 QMC5883L Compass (5-Pin Header, pitch=44px)
    gy271_pins = [
        ("VCC (3.3V)", "#EF4444"),
        ("GND", "#334155"),
        ("SCL", "#9333EA"),
        ("SDA", "#9333EA"),
        ("DRDY", "#64748B"),
    ]
    draw_sip_breakout(200, 1100, "GY-271 COMPASS (MAG 2)", "I2C2 BUS (Addr: 0x0D)", "#581C87", gy271_pins)

    # Module 3: BMP280 Barometer Breakout (4-Pin Header, pitch=44px)
    bmp280_pins = [
        ("VCC (3.3V)", "#EF4444"),
        ("GND", "#334155"),
        ("SCL", "#9333EA"),
        ("SDA", "#9333EA"),
    ]
    draw_sip_breakout(200, 850, "BMP280 BAROMETER", "I2C2 BUS (Addr: 0x76)", "#3B0764", bmp280_pins)

    # Module 4: ELRS / CRSF Nano Receiver (4-Pin Header, pitch=44px)
    rc_pins = [
        ("5V VCC", "#DC2626"),
        ("GND", "#334155"),
        ("TX (Telem)", "#0891B2"),
        ("RX (Channels)", "#0891B2"),
    ]
    draw_sip_breakout(200, 600, "ELRS / CRSF RECEIVER", "UART7 RADIO LINK (420k)", "#0E7490", rc_pins)

    # Module 5: Battery Voltage & Current Divider (3-Pin Header, pitch=44px)
    pwr_pins = [
        ("VOLT (11:1)", "#059669"),
        ("CURR (3.3V)", "#059669"),
        ("GND", "#334155"),
    ]
    draw_sip_breakout(550, 1720, "BATTERY MONITOR", "ADC1 VOLT & CURRENT", "#064E3B", pwr_pins)

    # Module 6: Motors 1–4 Output Terminal (5-Pin Header, pitch=44px)
    m1_4_pins = [
        ("M1 (PE9)", "#EA580C"),
        ("M2 (PE11)", "#EA580C"),
        ("M3 (PE13)", "#EA580C"),
        ("M4 (PE14)", "#EA580C"),
        ("GND", "#334155"),
    ]
    draw_sip_breakout(950, 1950, "MOTORS 1–4 (TIM1)", "QUAD / HEXA / ROVER LEFT", "#9A3412", m1_4_pins)

    # Module 7: Motors 5–8 Output Terminal (5-Pin Header, pitch=44px)
    m5_8_pins = [
        ("M5 (PC6)", "#EA580C"),
        ("M6 (PC7)", "#EA580C"),
        ("M7 (PB0)", "#EA580C"),
        ("M8 (PB1)", "#EA580C"),
        ("GND", "#334155"),
    ]
    draw_sip_breakout(2700, 1500, "MOTORS 5–8 (TIM3)", "HEXA / 8-WHEEL ROVER", "#9A3412", m5_8_pins)

    # Module 8: GPS Mast Puck & Compass Mag 1 (6-Pin Header, pitch=44px)
    gps_pins = [
        ("5V VCC", "#DC2626"),
        ("GND", "#334155"),
        ("GPS TX", "#0891B2"),
        ("GPS RX", "#0891B2"),
        ("MAG 1 SCL", "#7C3AED"),
        ("MAG 1 SDA", "#7C3AED"),
    ]
    draw_sip_breakout(2700, 750, "GPS & MAST COMPASS", "USART3 + I2C1 (PB8/PB9)", "#4C1D95", gps_pins)

    # Module 9: Raspberry Pi 4 / 5 Companion (3-Pin Header, pitch=44px)
    rpi_pins = [
        ("GPIO 14 (TX)", "#0891B2"),
        ("GPIO 15 (RX)", "#0891B2"),
        ("GND", "#334155"),
    ]
    draw_sip_breakout(2100, 350, "RASPBERRY PI 4 / 5", "USART2 @ 921,600 BAUD", "#155E75", rpi_pins)

    # Module 10: ESP32 Telemetry Module (4-Pin Header, pitch=44px)
    esp_pins = [
        ("5V VCC", "#DC2626"),
        ("GND", "#334155"),
        ("TXD0", "#0891B2"),
        ("RXD0", "#0891B2"),
    ]
    draw_sip_breakout(1600, 350, "ESP32 TELEMETRY", "UART4 @ 115,200 BAUD", "#164E63", esp_pins)

    # 5. Connect Wires from Component Pins to Exact Dot Holes
    def add_wire(src_id, target_hole_coord, stroke_color, label=""):
        if target_hole_coord not in hole_cells:
            return
        tgt_id = hole_cells[target_hole_coord]
        wire_id = next_id()
        wire_cell = ET.SubElement(root, 'mxCell', id=wire_id, parent="1", edge="1",
                                  source=src_id, target=tgt_id, value=label,
                                  style=f"edgeStyle=orthogonalEdgeStyle;rounded=1;orthogonalLoop=1;jettySize=auto;html=1;strokeColor={stroke_color};strokeWidth=2.2;fontSize=9;fontColor=#0F172A;labelBackgroundColor=#FFFFFF;")
        wg = ET.SubElement(wire_cell, 'mxGeometry', relative="1")
        wg.set('as', 'geometry')

    # BMI160 Wires -> Bank 1 Holes
    add_wire(component_pins[("BMI160 6-AXIS IMU", "VIN")], "W2", "#EF4444")
    add_wire(component_pins[("BMI160 6-AXIS IMU", "GND")], "V2", "#334155")
    add_wire(component_pins[("BMI160 6-AXIS IMU", "CS")], "I2", "#2563EB", "PA4 (CS)")
    add_wire(component_pins[("BMI160 6-AXIS IMU", "SCL (SCK)")], "I3", "#2563EB", "PA5 (SCK)")
    add_wire(component_pins[("BMI160 6-AXIS IMU", "SDA (MOSI)")], "J3", "#2563EB", "PA7 (MOSI)")
    add_wire(component_pins[("BMI160 6-AXIS IMU", "SAO (MISO)")], "J2", "#2563EB", "PA6 (MISO)")
    add_wire(component_pins[("BMI160 6-AXIS IMU", "INT1")], "C2", "#E11D48", "PE4 (INT1)")

    # Frame GY-271 Mag 2 Wires -> Bank 1 Holes (PB10 / PB11 on Col R)
    add_wire(component_pins[("GY-271 COMPASS (MAG 2)", "VCC (3.3V)")], "W3", "#EF4444")
    add_wire(component_pins[("GY-271 COMPASS (MAG 2)", "GND")], "V3", "#334155")
    add_wire(component_pins[("GY-271 COMPASS (MAG 2)", "SCL")], "R2", "#9333EA", "PB10 (SCL)")
    add_wire(component_pins[("GY-271 COMPASS (MAG 2)", "SDA")], "R3", "#9333EA", "PB11 (SDA)")

    # BMP280 Barometer Wires -> Bank 1 Holes (Shared I2C2 on Col R)
    add_wire(component_pins[("BMP280 BAROMETER", "VCC (3.3V)")], "W3", "#EF4444")
    add_wire(component_pins[("BMP280 BAROMETER", "GND")], "V3", "#334155")
    add_wire(component_pins[("BMP280 BAROMETER", "SCL")], "R2", "#9333EA", "PB10 (SCL)")
    add_wire(component_pins[("BMP280 BAROMETER", "SDA")], "R3", "#9333EA", "PB11 (SDA)")

    # RC Receiver Wires -> Bank 1 Holes (PE7 / PE8 on Col M and N)
    add_wire(component_pins[("ELRS / CRSF RECEIVER", "5V VCC")], "W16", "#DC2626")
    add_wire(component_pins[("ELRS / CRSF RECEIVER", "GND")], "V2", "#334155")
    add_wire(component_pins[("ELRS / CRSF RECEIVER", "TX (Telem)")], "N2", "#0891B2", "PE8 (TX)")
    add_wire(component_pins[("ELRS / CRSF RECEIVER", "RX (Channels)")], "M3", "#0891B2", "PE7 (RX)")

    # Battery Monitor -> Bank 1 Holes (PA0 on Col G, PC0 on Col E)
    add_wire(component_pins[("BATTERY MONITOR", "VOLT (11:1)")], "G2", "#059669", "PA0 (Volt)")
    add_wire(component_pins[("BATTERY MONITOR", "CURR (3.3V)")], "E2", "#059669", "PC0 (Curr)")
    add_wire(component_pins[("BATTERY MONITOR", "GND")], "V2", "#334155")

    # Motors 1-4 -> Bank 1 Holes (PE9, PE11, PE13, PE14 on Cols N, O, P, Q)
    add_wire(component_pins[("MOTORS 1–4 (TIM1)", "M1 (PE9)")], "N3", "#EA580C", "PE9 (M1)")
    add_wire(component_pins[("MOTORS 1–4 (TIM1)", "M2 (PE11)")], "O3", "#EA580C", "PE11 (M2)")
    add_wire(component_pins[("MOTORS 1–4 (TIM1)", "M3 (PE13)")], "P3", "#EA580C", "PE13 (M3)")
    add_wire(component_pins[("MOTORS 1–4 (TIM1)", "M4 (PE14)")], "Q2", "#EA580C", "PE14 (M4)")
    add_wire(component_pins[("MOTORS 1–4 (TIM1)", "GND")], "V2", "#334155")

    # GPS & Mast Compass (Mag 1) -> Bank 2 Holes (USART3 on Col T, I2C1 on Col C)
    add_wire(component_pins[("GPS & MAST COMPASS", "5V VCC")], "W17", "#DC2626")
    add_wire(component_pins[("GPS & MAST COMPASS", "GND")], "U17", "#334155")
    add_wire(component_pins[("GPS & MAST COMPASS", "GPS TX")], "T17", "#0891B2", "PD9 (GPS TX)")
    add_wire(component_pins[("GPS & MAST COMPASS", "GPS RX")], "T16", "#0891B2", "PD8 (GPS RX)")
    add_wire(component_pins[("GPS & MAST COMPASS", "MAG 1 SCL")], "C16", "#7C3AED", "PB8 (SCL)")
    add_wire(component_pins[("GPS & MAST COMPASS", "MAG 1 SDA")], "C17", "#7C3AED", "PB9 (SDA)")

    # Raspberry Pi -> Bank 2 Holes (USART2 on Col G: PD6, PD5)
    add_wire(component_pins[("RASPBERRY PI 4 / 5", "GPIO 14 (TX)")], "G17", "#0891B2", "PD6 (RPi TX)")
    add_wire(component_pins[("RASPBERRY PI 4 / 5", "GPIO 15 (RX)")], "G16", "#0891B2", "PD5 (RPi RX)")
    add_wire(component_pins[("RASPBERRY PI 4 / 5", "GND")], "U16", "#334155")

    # ESP32 Telemetry -> Bank 2 Holes (UART4 on Col J and I: PD0, PD1)
    add_wire(component_pins[("ESP32 TELEMETRY", "5V VCC")], "W16", "#DC2626")
    add_wire(component_pins[("ESP32 TELEMETRY", "GND")], "U16", "#334155")
    add_wire(component_pins[("ESP32 TELEMETRY", "TXD0")], "J17", "#0891B2", "PD0 (ESP TX)")
    add_wire(component_pins[("ESP32 TELEMETRY", "RXD0")], "I16", "#0891B2", "PD1 (ESP RX)")

    # Motors 5-8 -> Bank 2 (M5, M6 on Col P) & Bank 1 (M7, M8 on Col L)
    add_wire(component_pins[("MOTORS 5–8 (TIM3)", "M5 (PC6)")], "P16", "#EA580C", "PC6 (M5)")
    add_wire(component_pins[("MOTORS 5–8 (TIM3)", "M6 (PC7)")], "P17", "#EA580C", "PC7 (M6)")
    add_wire(component_pins[("MOTORS 5–8 (TIM3)", "M7 (PB0)")], "L2", "#EA580C", "PB0 (M7)")
    add_wire(component_pins[("MOTORS 5–8 (TIM3)", "M8 (PB1)")], "L3", "#EA580C", "PB1 (M8)")
    add_wire(component_pins[("MOTORS 5–8 (TIM3)", "GND")], "V2", "#334155")

    # Pretty print XML
    xml_str = ET.tostring(mxfile, encoding='utf-8')
    dom = minidom.parseString(xml_str)
    pretty_xml = dom.toprettyxml(indent="  ")

    output_drawio = "Hardware/schematics/dotboard_circuit.drawio"
    output_xml = "Hardware/schematics/dotboard_circuit.xml"
    with open(output_drawio, "w", encoding="utf-8") as f:
        f.write(pretty_xml)
    with open(output_xml, "w", encoding="utf-8") as f:
        f.write(pretty_xml)
    print(f"Generated {output_drawio} and {output_xml} successfully ({len(pretty_xml)} bytes)")

if __name__ == "__main__":
    create_drawio_xml()
