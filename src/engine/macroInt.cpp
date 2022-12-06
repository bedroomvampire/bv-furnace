/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "macroInt.h"
#include "instrument.h"
#include "engine.h"

#define ADSR_LOW source.val[0]
#define ADSR_HIGH source.val[1]
#define ADSR_AR source.val[2]
#define ADSR_HT source.val[3]
#define ADSR_DR source.val[4]
#define ADSR_SL source.val[5]
#define ADSR_ST source.val[6]
#define ADSR_SR source.val[7]
#define ADSR_RR source.val[8]

#define LFO_SPEED source.val[11]
#define LFO_WAVE source.val[12]
#define LFO_PHASE source.val[13]
#define LFO_LOOP source.val[14]
#define LFO_GLOBAL source.val[15]

void DivMacroStruct::prepare(DivInstrumentMacro& source, DivEngine* e) {
  has=had=actualHad=will=true;
  mode=source.mode;
  type=(source.open>>1)&3;
  linger=(source.name=="vol" && e->song.volMacroLinger);
  lfoPos=LFO_PHASE;
}

void DivMacroStruct::doMacro(DivInstrumentMacro& source, bool released, bool tick) {
  if (!tick) {
    had=false;
    return;
  }
  if (delay>0) {
    delay--;
    had=false;
    return;
  }
  if (began && source.delay>0) {
    delay=source.delay;
  } else {
    delay=source.speed-1;
  }
  if (began) {
    began=false;
  }
  if (finished) {
    finished=false;
  }
  if (actualHad!=has) {
    finished=true;
  }
  actualHad=has;
  had=actualHad;

  if (has) {
<<<<<<< HEAD
    if (type==0) { // sequence
      lastPos=pos;
      val=source.val[pos++];
      if (pos>source.rel && !released) {
        if (source.loop<source.len && source.loop<source.rel) {
          pos=source.loop;
        } else {
          pos--;
        }
      }
      if (pos>=source.len) {
        if (source.loop<source.len && (source.loop>=source.rel || source.rel>=source.len)) {
          pos=source.loop;
        } else if (linger) {
          pos--;
        } else {
          has=false;
        }
=======
    val=source.val[pos++];
    if (source.rel>=0 && pos>source.rel && !released) {
      if (source.loop<source.len && source.loop>=0 && source.loop<source.rel) {
        pos=source.loop;
      } else {
        pos--;
>>>>>>> parent of f01be34b (GUI: highlight current macro position)
      }
    }
    if (type==1) { // ADSR
      if (released && lastPos<3) lastPos=3;
      switch (lastPos) {
        case 0: // attack
          pos+=ADSR_AR;
          if (pos>255) {
            pos=255;
            lastPos=1;
            delay=ADSR_HT;
          }
          break;
        case 1: // decay
          pos-=ADSR_DR;
          if (pos<=ADSR_SL) {
            pos=ADSR_SL;
            lastPos=2;
            delay=ADSR_ST;
          }
          break;
        case 2: // sustain
          pos-=ADSR_SR;
          if (pos<0) {
            pos=0;
            lastPos=4;
          }
          break;
        case 3: // release
          pos-=ADSR_RR;
          if (pos<0) {
            pos=0;
            lastPos=4;
          }
          break;
        case 4: // end
          pos=0;
          if (!linger) has=false;
          break;
      }
      val=ADSR_LOW+((pos+(ADSR_HIGH-ADSR_LOW)*pos)>>8);
    }
    if (type==2) { // LFO
      lfoPos+=LFO_SPEED;
      lfoPos&=1023;

      int lfoOut=0;
      switch (LFO_WAVE&3) {
        case 0: // triangle
          lfoOut=((lfoPos&512)?(1023-lfoPos):(lfoPos))>>1;
          break;
        case 1: // saw
          lfoOut=lfoPos>>2;
          break;
        case 2: // pulse
          lfoOut=(lfoPos&512)?255:0;
          break;
      }
      val=ADSR_LOW+((lfoOut+(ADSR_HIGH-ADSR_LOW)*lfoOut)>>8);
    }
  }
}

void DivMacroInt::next() {
  if (ins==NULL) return;
  // run macros
  // TODO: potentially get rid of list to avoid allocations
  subTick--;
  for (size_t i=0; i<macroListLen; i++) {
    if (macroList[i]!=NULL && macroSource[i]!=NULL) {
      macroList[i]->doMacro(*macroSource[i],released,subTick==0);
    }
  }
  if (subTick<=0) {
    if (e==NULL) {
      subTick=1;
    } else {
      subTick=e->tickMult;
    }
  }
}

void DivMacroInt::release() {
  released=true;
}

void DivMacroInt::setEngine(DivEngine* eng) {
  e=eng;
}

#define ADD_MACRO(m,s) \
  macroList[macroListLen]=&m; \
  macroSource[macroListLen++]=&s;

void DivMacroInt::init(DivInstrument* which) {
  ins=which;
  // initialize
  for (size_t i=0; i<macroListLen; i++) {
    if (macroList[i]!=NULL) macroList[i]->init();
  }
  macroListLen=0;
  subTick=1;

  hasRelease=false;
  released=false;

  if (ins==NULL) return;

  // prepare common macro
  if (ins->std.volMacro.len>0) {
    ADD_MACRO(vol,ins->std.volMacro);
  }
  if (ins->std.arpMacro.len>0) {
    ADD_MACRO(arp,ins->std.arpMacro);
  }
  if (ins->std.dutyMacro.len>0) {
    ADD_MACRO(duty,ins->std.dutyMacro);
  }
  if (ins->std.waveMacro.len>0) {
    ADD_MACRO(wave,ins->std.waveMacro);
  }
  if (ins->std.pitchMacro.len>0) {
    ADD_MACRO(pitch,ins->std.pitchMacro);
  }
  if (ins->std.ex1Macro.len>0) {
    ADD_MACRO(ex1,ins->std.ex1Macro);
  }
  if (ins->std.ex2Macro.len>0) {
    ADD_MACRO(ex2,ins->std.ex2Macro);
  }
  if (ins->std.ex3Macro.len>0) {
    ADD_MACRO(ex3,ins->std.ex3Macro);
  }
  if (ins->std.algMacro.len>0) {
    ADD_MACRO(alg,ins->std.algMacro);
  }
  if (ins->std.fbMacro.len>0) {
    ADD_MACRO(fb,ins->std.fbMacro);
  }
  if (ins->std.fmsMacro.len>0) {
    ADD_MACRO(fms,ins->std.fmsMacro);
  }
  if (ins->std.amsMacro.len>0) {
    ADD_MACRO(ams,ins->std.amsMacro);
  }

  if (ins->std.panLMacro.len>0) {
    ADD_MACRO(panL,ins->std.panLMacro);
  }
  if (ins->std.panRMacro.len>0) {
    ADD_MACRO(panR,ins->std.panRMacro);
  }
  if (ins->std.phaseResetMacro.len>0) {
    ADD_MACRO(phaseReset,ins->std.phaseResetMacro);
  }
  if (ins->std.ex4Macro.len>0) {
    ADD_MACRO(ex4,ins->std.ex4Macro);
  }
  if (ins->std.ex5Macro.len>0) {
    ADD_MACRO(ex5,ins->std.ex5Macro);
  }
  if (ins->std.ex6Macro.len>0) {
    ADD_MACRO(ex6,ins->std.ex6Macro);
  }
  if (ins->std.ex7Macro.len>0) {
    ADD_MACRO(ex7,ins->std.ex7Macro);
  }
  if (ins->std.ex8Macro.len>0) {
    ADD_MACRO(ex8,ins->std.ex8Macro);
  }

  // prepare FM operator macros
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& m=ins->std.opMacros[i];
    IntOp& o=op[i];
    if (m.amMacro.len>0) {
      ADD_MACRO(o.am,m.amMacro);
    }
    if (m.arMacro.len>0) {
      ADD_MACRO(o.ar,m.arMacro);
    }
    if (m.drMacro.len>0) {
      ADD_MACRO(o.dr,m.drMacro);
    }
    if (m.multMacro.len>0) {
      ADD_MACRO(o.mult,m.multMacro);
    }
    if (m.rrMacro.len>0) {
      ADD_MACRO(o.rr,m.rrMacro);
    }
    if (m.slMacro.len>0) {
      ADD_MACRO(o.sl,m.slMacro);
    }
    if (m.tlMacro.len>0) {
      ADD_MACRO(o.tl,m.tlMacro);
    }
    if (m.dt2Macro.len>0) {
      ADD_MACRO(o.dt2,m.dt2Macro);
    }
    if (m.rsMacro.len>0) {
      ADD_MACRO(o.rs,m.rsMacro);
    }
    if (m.dtMacro.len>0) {
      ADD_MACRO(o.dt,m.dtMacro);
    }
    if (m.d2rMacro.len>0) {
      ADD_MACRO(o.d2r,m.d2rMacro);
    }
    if (m.ssgMacro.len>0) {
      ADD_MACRO(o.ssg,m.ssgMacro);
    }

    if (m.damMacro.len>0) {
      ADD_MACRO(o.dam,m.damMacro);
    }
    if (m.dvbMacro.len>0) {
      ADD_MACRO(o.dvb,m.dvbMacro);
    }
    if (m.egtMacro.len>0) {
      ADD_MACRO(o.egt,m.egtMacro);
    }
    if (m.kslMacro.len>0) {
      ADD_MACRO(o.ksl,m.kslMacro);
    }
    if (m.susMacro.len>0) {
      ADD_MACRO(o.sus,m.susMacro);
    }
    if (m.vibMacro.len>0) {
      ADD_MACRO(o.vib,m.vibMacro);
    }
    if (m.wsMacro.len>0) {
      ADD_MACRO(o.ws,m.wsMacro);
    }
    if (m.ksrMacro.len>0) {
      ADD_MACRO(o.ksr,m.ksrMacro);
    }
  }

  for (size_t i=0; i<macroListLen; i++) {
    if (macroSource[i]!=NULL) {
      macroList[i]->prepare(*macroSource[i],e);
      // check ADSR mode
      if ((macroSource[i]->open&6)==4) {
        hasRelease=false;
      } else if ((macroSource[i]->open&6)==2) {
        hasRelease=true;
      } else {
        hasRelease=(macroSource[i]->rel<macroSource[i]->len);
      }
    } else {
      hasRelease=false;
    }
  }
}

void DivMacroInt::notifyInsDeletion(DivInstrument* which) {
  if (ins==which) {
    init(NULL);
  }
}
