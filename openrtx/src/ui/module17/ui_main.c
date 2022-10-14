/***************************************************************************
 *   Copyright (C) 2020 - 2022 by Federico Amedeo Izzo IU2NUO,             *
 *                                Niccolò Izzo IU2KIN                      *
 *                                Frederik Saraci IU2NRO                   *
 *                                Silvano Seva IU2KWO                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <interfaces/platform.h>
#include <interfaces/cps_io.h>
#include <stdio.h>
#include <stdint.h>
#include <ui.h>
#include <string.h>

void _ui_drawMainBackground()
{
    // Print top bar line of hline_h pixel height
    gfx_drawHLine(layout.top_h, layout.hline_h, color_grey);
    // Print bottom bar line of 1 pixel height
    gfx_drawHLine(SCREEN_HEIGHT - layout.bottom_h - 1, layout.hline_h, color_grey);
}

void _ui_drawMainTop()
{
	 gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_LEFT,
			color_white, last_state.settings.callsign);
				  
	gfx_print(layout.top_pos, layout.top_font, TEXT_ALIGN_RIGHT,
			color_white,"%.1fV", 13.8);
}

void _ui_drawBankChannel()
{
    // Print Bank number, channel number and Channel name
    uint16_t b = (last_state.bank_enabled) ? last_state.bank : 0;
    gfx_print(layout.line1_pos, layout.line1_font, TEXT_ALIGN_CENTER,
              color_white, "%01d-%03d: %.12s",
              b, last_state.channel_index + 1, last_state.channel.name);
}

void _ui_drawModeInfo(ui_state_t* ui_state)
{
    char bw_str[8] = { 0 };
    char encdec_str[9] = { 0 };

    rtxStatus_t cfg = rtx_getCurrentStatus();

    switch(last_state.channel.mode)
    {
        case OPMODE_FM:
        // Get Bandwidth string
        if(last_state.channel.bandwidth == BW_12_5)
            snprintf(bw_str, 8, "12.5");
        else if(last_state.channel.bandwidth == BW_20)
            snprintf(bw_str, 8, "20");
        else if(last_state.channel.bandwidth == BW_25)
            snprintf(bw_str, 8, "25");
        // Get encdec string
        bool tone_tx_enable = last_state.channel.fm.txToneEn;
        bool tone_rx_enable = last_state.channel.fm.rxToneEn;
        if (tone_tx_enable && tone_rx_enable)
            snprintf(encdec_str, 9, "E+D");
        else if (tone_tx_enable && !tone_rx_enable)
            snprintf(encdec_str, 9, "E");
        else if (!tone_tx_enable && tone_rx_enable)
            snprintf(encdec_str, 9, "D");
        else
            snprintf(encdec_str, 9, " ");

        // Print Bandwidth, Tone and encdec info
        gfx_print(layout.line2_pos, layout.line2_font, TEXT_ALIGN_CENTER,
              color_white, "B:%s T:%4.1f S:%s",
              bw_str, ctcss_tone[last_state.channel.fm.txTone]/10.0f,
              encdec_str);
        break;
        case OPMODE_DMR:
        // Print talkgroup
        gfx_print(layout.line2_pos, layout.line2_font, TEXT_ALIGN_CENTER,
              color_white, "TG:%s",
              "");
        break;
        case OPMODE_M17:
        {
            // Print M17 Destination ID on line 3 of 3
            char *dst = NULL;
            if(ui_state->edit_mode)
                dst = ui_state->new_callsign;
            else
                dst = (!strnlen(cfg.destination_address, 10)) ?
                    "Broadcast" : cfg.destination_address;
            gfx_print(layout.line2_pos, layout.line2_font, TEXT_ALIGN_CENTER,
                  color_white, "#%s", dst);
            break;
        }
    }
}

void _ui_drawFrequency()
{
  unsigned long frequency = platform_getPttStatus() ?
       frequency = last_state.channel.tx_frequency : last_state.channel.rx_frequency;

    // Print big numbers frequency
    gfx_print(layout.line2_pos, FONT_SIZE_7PT, TEXT_ALIGN_LEFT,
              color_white, "DST: BROADCAST");
    gfx_print(layout.line3_pos, FONT_SIZE_6PT, TEXT_ALIGN_LEFT,
              color_white, "CAN: 0");          
}

void _ui_drawVFOMiddleInput(ui_state_t* ui_state)
{
    // Add inserted number to string, skipping "Rx: "/"Tx: " and "."
    uint8_t insert_pos = ui_state->input_position + 3;
    if(ui_state->input_position > 3) insert_pos += 1;
    char input_char = ui_state->input_number + '0';

    if(ui_state->input_set == SET_RX)
    {
        if(ui_state->input_position == 0)
        {
            gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ">Rx:%03lu.%04lu",
                      (unsigned long)ui_state->new_rx_frequency/1000000,
                      (unsigned long)(ui_state->new_rx_frequency%1000000)/100);
        }
        else
        {
            // Replace Rx frequency with underscorses
            if(ui_state->input_position == 1)
                strcpy(ui_state->new_rx_freq_buf, ">Rx:___.____");
            ui_state->new_rx_freq_buf[insert_pos] = input_char;
            gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ui_state->new_rx_freq_buf);
        }
        gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
                  color_white, " Tx:%03lu.%04lu",
                  (unsigned long)last_state.channel.tx_frequency/1000000,
                  (unsigned long)(last_state.channel.tx_frequency%1000000)/100);
    }
    else if(ui_state->input_set == SET_TX)
    {
        gfx_print(layout.line2_pos, layout.input_font, TEXT_ALIGN_CENTER,
                  color_white, " Rx:%03lu.%04lu",
                  (unsigned long)ui_state->new_rx_frequency/1000000,
                  (unsigned long)(ui_state->new_rx_frequency%1000000)/100);
        // Replace Rx frequency with underscorses
        if(ui_state->input_position == 0)
        {
            gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ">Tx:%03lu.%04lu",
                      (unsigned long)ui_state->new_rx_frequency/1000000,
                      (unsigned long)(ui_state->new_rx_frequency%1000000)/100);
        }
        else
        {
            if(ui_state->input_position == 1)
                strcpy(ui_state->new_tx_freq_buf, ">Tx:___.____");
            ui_state->new_tx_freq_buf[insert_pos] = input_char;
            gfx_print(layout.line3_pos, layout.input_font, TEXT_ALIGN_CENTER,
                      color_white, ui_state->new_tx_freq_buf);
        }
    }
}

void _ui_drawMainBottom()
{
    gfx_print(layout.bottom_pos, layout.menu_font, TEXT_ALIGN_CENTER,
                color_white, "DST");
    gfx_print(layout.bottom_pos, layout.menu_font, TEXT_ALIGN_RIGHT,
                color_white, "Menu");     
}

void _ui_drawMainVFO(ui_state_t* ui_state)
{
    gfx_clearScreen();
    _ui_drawMainTop();
    _ui_drawModeInfo(ui_state);
    _ui_drawFrequency();
    _ui_drawMainBottom();
}

void _ui_drawMainVFOInput(ui_state_t* ui_state)
{
    gfx_clearScreen();
    _ui_drawMainTop();
    _ui_drawVFOMiddleInput(ui_state);
    _ui_drawMainBottom();
}

void _ui_drawMainMEM(ui_state_t* ui_state)
{
    gfx_clearScreen();
    _ui_drawMainTop();
    _ui_drawBankChannel();
    _ui_drawModeInfo(ui_state);
    _ui_drawFrequency();
    _ui_drawMainBottom();
}
