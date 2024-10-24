/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -fno-inline %s | %FileCheck %s
// RUN: %hermes -O0 -fno-inline %s | %FileCheck %s

// Exercises bug caused when there are too many registers, and RemoveGenerator uses
// a "long" register (above r255).

var print = typeof print !== "undefined" ? print : console.log;

function sink(v) { return v; }

function *gen() {
  var _0 = sink(1);
  var _1 = sink(_0 + 1);
  var _2 = sink(_1 + 1);
  var _3 = sink(_2 + 1);
  var _4 = sink(_3 + 1);
  var _5 = sink(_4 + 1);
  var _6 = sink(_5 + 1);
  var _7 = sink(_6 + 1);
  var _8 = sink(_7 + 1);
  var _9 = sink(_8 + 1);
  var _10 = sink(_9 + 1);
  var _11 = sink(_10 + 1);
  var _12 = sink(_11 + 1);
  var _13 = sink(_12 + 1);
  var _14 = sink(_13 + 1);
  var _15 = sink(_14 + 1);
  var _16 = sink(_15 + 1);
  var _17 = sink(_16 + 1);
  var _18 = sink(_17 + 1);
  var _19 = sink(_18 + 1);
  var _20 = sink(_19 + 1);
  var _21 = sink(_20 + 1);
  var _22 = sink(_21 + 1);
  var _23 = sink(_22 + 1);
  var _24 = sink(_23 + 1);
  var _25 = sink(_24 + 1);
  var _26 = sink(_25 + 1);
  var _27 = sink(_26 + 1);
  var _28 = sink(_27 + 1);
  var _29 = sink(_28 + 1);
  var _30 = sink(_29 + 1);
  var _31 = sink(_30 + 1);
  var _32 = sink(_31 + 1);
  var _33 = sink(_32 + 1);
  var _34 = sink(_33 + 1);
  var _35 = sink(_34 + 1);
  var _36 = sink(_35 + 1);
  var _37 = sink(_36 + 1);
  var _38 = sink(_37 + 1);
  var _39 = sink(_38 + 1);
  var _40 = sink(_39 + 1);
  var _41 = sink(_40 + 1);
  var _42 = sink(_41 + 1);
  var _43 = sink(_42 + 1);
  var _44 = sink(_43 + 1);
  var _45 = sink(_44 + 1);
  var _46 = sink(_45 + 1);
  var _47 = sink(_46 + 1);
  var _48 = sink(_47 + 1);
  var _49 = sink(_48 + 1);
  var _50 = sink(_49 + 1);
  var _51 = sink(_50 + 1);
  var _52 = sink(_51 + 1);
  var _53 = sink(_52 + 1);
  var _54 = sink(_53 + 1);
  var _55 = sink(_54 + 1);
  var _56 = sink(_55 + 1);
  var _57 = sink(_56 + 1);
  var _58 = sink(_57 + 1);
  var _59 = sink(_58 + 1);
  var _60 = sink(_59 + 1);
  var _61 = sink(_60 + 1);
  var _62 = sink(_61 + 1);
  var _63 = sink(_62 + 1);
  var _64 = sink(_63 + 1);
  var _65 = sink(_64 + 1);
  var _66 = sink(_65 + 1);
  var _67 = sink(_66 + 1);
  var _68 = sink(_67 + 1);
  var _69 = sink(_68 + 1);
  var _70 = sink(_69 + 1);
  var _71 = sink(_70 + 1);
  var _72 = sink(_71 + 1);
  var _73 = sink(_72 + 1);
  var _74 = sink(_73 + 1);
  var _75 = sink(_74 + 1);
  var _76 = sink(_75 + 1);
  var _77 = sink(_76 + 1);
  var _78 = sink(_77 + 1);
  var _79 = sink(_78 + 1);
  var _80 = sink(_79 + 1);
  var _81 = sink(_80 + 1);
  var _82 = sink(_81 + 1);
  var _83 = sink(_82 + 1);
  var _84 = sink(_83 + 1);
  var _85 = sink(_84 + 1);
  var _86 = sink(_85 + 1);
  var _87 = sink(_86 + 1);
  var _88 = sink(_87 + 1);
  var _89 = sink(_88 + 1);
  var _90 = sink(_89 + 1);
  var _91 = sink(_90 + 1);
  var _92 = sink(_91 + 1);
  var _93 = sink(_92 + 1);
  var _94 = sink(_93 + 1);
  var _95 = sink(_94 + 1);
  var _96 = sink(_95 + 1);
  var _97 = sink(_96 + 1);
  var _98 = sink(_97 + 1);
  var _99 = sink(_98 + 1);
  var _100 = sink(_99 + 1);
  var _101 = sink(_100 + 1);
  var _102 = sink(_101 + 1);
  var _103 = sink(_102 + 1);
  var _104 = sink(_103 + 1);
  var _105 = sink(_104 + 1);
  var _106 = sink(_105 + 1);
  var _107 = sink(_106 + 1);
  var _108 = sink(_107 + 1);
  var _109 = sink(_108 + 1);
  var _110 = sink(_109 + 1);
  var _111 = sink(_110 + 1);
  var _112 = sink(_111 + 1);
  var _113 = sink(_112 + 1);
  var _114 = sink(_113 + 1);
  var _115 = sink(_114 + 1);
  var _116 = sink(_115 + 1);
  var _117 = sink(_116 + 1);
  var _118 = sink(_117 + 1);
  var _119 = sink(_118 + 1);
  var _120 = sink(_119 + 1);
  var _121 = sink(_120 + 1);
  var _122 = sink(_121 + 1);
  var _123 = sink(_122 + 1);
  var _124 = sink(_123 + 1);
  var _125 = sink(_124 + 1);
  var _126 = sink(_125 + 1);
  var _127 = sink(_126 + 1);
  var _128 = sink(_127 + 1);
  var _129 = sink(_128 + 1);
  var _130 = sink(_129 + 1);
  var _131 = sink(_130 + 1);
  var _132 = sink(_131 + 1);
  var _133 = sink(_132 + 1);
  var _134 = sink(_133 + 1);
  var _135 = sink(_134 + 1);
  var _136 = sink(_135 + 1);
  var _137 = sink(_136 + 1);
  var _138 = sink(_137 + 1);
  var _139 = sink(_138 + 1);
  var _140 = sink(_139 + 1);
  var _141 = sink(_140 + 1);
  var _142 = sink(_141 + 1);
  var _143 = sink(_142 + 1);
  var _144 = sink(_143 + 1);
  var _145 = sink(_144 + 1);
  var _146 = sink(_145 + 1);
  var _147 = sink(_146 + 1);
  var _148 = sink(_147 + 1);
  var _149 = sink(_148 + 1);
  var _150 = sink(_149 + 1);
  var _151 = sink(_150 + 1);
  var _152 = sink(_151 + 1);
  var _153 = sink(_152 + 1);
  var _154 = sink(_153 + 1);
  var _155 = sink(_154 + 1);
  var _156 = sink(_155 + 1);
  var _157 = sink(_156 + 1);
  var _158 = sink(_157 + 1);
  var _159 = sink(_158 + 1);
  var _160 = sink(_159 + 1);
  var _161 = sink(_160 + 1);
  var _162 = sink(_161 + 1);
  var _163 = sink(_162 + 1);
  var _164 = sink(_163 + 1);
  var _165 = sink(_164 + 1);
  var _166 = sink(_165 + 1);
  var _167 = sink(_166 + 1);
  var _168 = sink(_167 + 1);
  var _169 = sink(_168 + 1);
  var _170 = sink(_169 + 1);
  var _171 = sink(_170 + 1);
  var _172 = sink(_171 + 1);
  var _173 = sink(_172 + 1);
  var _174 = sink(_173 + 1);
  var _175 = sink(_174 + 1);
  var _176 = sink(_175 + 1);
  var _177 = sink(_176 + 1);
  var _178 = sink(_177 + 1);
  var _179 = sink(_178 + 1);
  var _180 = sink(_179 + 1);
  var _181 = sink(_180 + 1);
  var _182 = sink(_181 + 1);
  var _183 = sink(_182 + 1);
  var _184 = sink(_183 + 1);
  var _185 = sink(_184 + 1);
  var _186 = sink(_185 + 1);
  var _187 = sink(_186 + 1);
  var _188 = sink(_187 + 1);
  var _189 = sink(_188 + 1);
  var _190 = sink(_189 + 1);
  var _191 = sink(_190 + 1);
  var _192 = sink(_191 + 1);
  var _193 = sink(_192 + 1);
  var _194 = sink(_193 + 1);
  var _195 = sink(_194 + 1);
  var _196 = sink(_195 + 1);
  var _197 = sink(_196 + 1);
  var _198 = sink(_197 + 1);
  var _199 = sink(_198 + 1);
  var _200 = sink(_199 + 1);
  var _201 = sink(_200 + 1);
  var _202 = sink(_201 + 1);
  var _203 = sink(_202 + 1);
  var _204 = sink(_203 + 1);
  var _205 = sink(_204 + 1);
  var _206 = sink(_205 + 1);
  var _207 = sink(_206 + 1);
  var _208 = sink(_207 + 1);
  var _209 = sink(_208 + 1);
  var _210 = sink(_209 + 1);
  var _211 = sink(_210 + 1);
  var _212 = sink(_211 + 1);
  var _213 = sink(_212 + 1);
  var _214 = sink(_213 + 1);
  var _215 = sink(_214 + 1);
  var _216 = sink(_215 + 1);
  var _217 = sink(_216 + 1);
  var _218 = sink(_217 + 1);
  var _219 = sink(_218 + 1);
  var _220 = sink(_219 + 1);
  var _221 = sink(_220 + 1);
  var _222 = sink(_221 + 1);
  var _223 = sink(_222 + 1);
  var _224 = sink(_223 + 1);
  var _225 = sink(_224 + 1);
  var _226 = sink(_225 + 1);
  var _227 = sink(_226 + 1);
  var _228 = sink(_227 + 1);
  var _229 = sink(_228 + 1);
  var _230 = sink(_229 + 1);
  var _231 = sink(_230 + 1);
  var _232 = sink(_231 + 1);
  var _233 = sink(_232 + 1);
  var _234 = sink(_233 + 1);
  var _235 = sink(_234 + 1);
  var _236 = sink(_235 + 1);
  var _237 = sink(_236 + 1);
  var _238 = sink(_237 + 1);
  var _239 = sink(_238 + 1);
  var _240 = sink(_239 + 1);
  var _241 = sink(_240 + 1);
  var _242 = sink(_241 + 1);
  var _243 = sink(_242 + 1);
  var _244 = sink(_243 + 1);
  var _245 = sink(_244 + 1);
  var _246 = sink(_245 + 1);
  var _247 = sink(_246 + 1);
  var _248 = sink(_247 + 1);
  var _249 = sink(_248 + 1);
  var _250 = sink(_249 + 1);
  var _251 = sink(_250 + 1);
  var _252 = sink(_251 + 1);
  var _253 = sink(_252 + 1);
  var _254 = sink(_253 + 1);
  var _255 = sink(_254 + 1);
  var _256 = sink(_255 + 1);
  var _257 = sink(_256 + 1);
  var _258 = sink(_257 + 1);
  var _259 = sink(_258 + 1);
  var _260 = sink(_259 + 1);
  var _261 = sink(_260 + 1);
  var _262 = sink(_261 + 1);
  var _263 = sink(_262 + 1);
  var _264 = sink(_263 + 1);
  var _265 = sink(_264 + 1);
  var _266 = sink(_265 + 1);
  var _267 = sink(_266 + 1);
  var _268 = sink(_267 + 1);
  var _269 = sink(_268 + 1);
  var _270 = sink(_269 + 1);
  var _271 = sink(_270 + 1);
  var _272 = sink(_271 + 1);
  var _273 = sink(_272 + 1);
  var _274 = sink(_273 + 1);
  var _275 = sink(_274 + 1);
  var _276 = sink(_275 + 1);
  var _277 = sink(_276 + 1);
  var _278 = sink(_277 + 1);
  var _279 = sink(_278 + 1);
  var _280 = sink(_279 + 1);
  var _281 = sink(_280 + 1);
  var _282 = sink(_281 + 1);
  var _283 = sink(_282 + 1);
  var _284 = sink(_283 + 1);
  var _285 = sink(_284 + 1);
  var _286 = sink(_285 + 1);
  var _287 = sink(_286 + 1);
  var _288 = sink(_287 + 1);
  var _289 = sink(_288 + 1);
  var _290 = sink(_289 + 1);
  var _291 = sink(_290 + 1);
  var _292 = sink(_291 + 1);
  var _293 = sink(_292 + 1);
  var _294 = sink(_293 + 1);
  var _295 = sink(_294 + 1);
  var _296 = sink(_295 + 1);
  var _297 = sink(_296 + 1);
  var _298 = sink(_297 + 1);
  var _299 = sink(_298 + 1);
  var _300 = sink(_299 + 1);
  yield _0;
  yield _1;
  yield _2;
  yield _3;
  yield _4;
  yield _5;
  yield _6;
  yield _7;
  yield _8;
  yield _9;
  yield _10;
  yield _11;
  yield _12;
  yield _13;
  yield _14;
  yield _15;
  yield _16;
  yield _17;
  yield _18;
  yield _19;
  yield _20;
  yield _21;
  yield _22;
  yield _23;
  yield _24;
  yield _25;
  yield _26;
  yield _27;
  yield _28;
  yield _29;
  yield _30;
  yield _31;
  yield _32;
  yield _33;
  yield _34;
  yield _35;
  yield _36;
  yield _37;
  yield _38;
  yield _39;
  yield _40;
  yield _41;
  yield _42;
  yield _43;
  yield _44;
  yield _45;
  yield _46;
  yield _47;
  yield _48;
  yield _49;
  yield _50;
  yield _51;
  yield _52;
  yield _53;
  yield _54;
  yield _55;
  yield _56;
  yield _57;
  yield _58;
  yield _59;
  yield _60;
  yield _61;
  yield _62;
  yield _63;
  yield _64;
  yield _65;
  yield _66;
  yield _67;
  yield _68;
  yield _69;
  yield _70;
  yield _71;
  yield _72;
  yield _73;
  yield _74;
  yield _75;
  yield _76;
  yield _77;
  yield _78;
  yield _79;
  yield _80;
  yield _81;
  yield _82;
  yield _83;
  yield _84;
  yield _85;
  yield _86;
  yield _87;
  yield _88;
  yield _89;
  yield _90;
  yield _91;
  yield _92;
  yield _93;
  yield _94;
  yield _95;
  yield _96;
  yield _97;
  yield _98;
  yield _99;
  yield _100;
  yield _101;
  yield _102;
  yield _103;
  yield _104;
  yield _105;
  yield _106;
  yield _107;
  yield _108;
  yield _109;
  yield _110;
  yield _111;
  yield _112;
  yield _113;
  yield _114;
  yield _115;
  yield _116;
  yield _117;
  yield _118;
  yield _119;
  yield _120;
  yield _121;
  yield _122;
  yield _123;
  yield _124;
  yield _125;
  yield _126;
  yield _127;
  yield _128;
  yield _129;
  yield _130;
  yield _131;
  yield _132;
  yield _133;
  yield _134;
  yield _135;
  yield _136;
  yield _137;
  yield _138;
  yield _139;
  yield _140;
  yield _141;
  yield _142;
  yield _143;
  yield _144;
  yield _145;
  yield _146;
  yield _147;
  yield _148;
  yield _149;
  yield _150;
  yield _151;
  yield _152;
  yield _153;
  yield _154;
  yield _155;
  yield _156;
  yield _157;
  yield _158;
  yield _159;
  yield _160;
  yield _161;
  yield _162;
  yield _163;
  yield _164;
  yield _165;
  yield _166;
  yield _167;
  yield _168;
  yield _169;
  yield _170;
  yield _171;
  yield _172;
  yield _173;
  yield _174;
  yield _175;
  yield _176;
  yield _177;
  yield _178;
  yield _179;
  yield _180;
  yield _181;
  yield _182;
  yield _183;
  yield _184;
  yield _185;
  yield _186;
  yield _187;
  yield _188;
  yield _189;
  yield _190;
  yield _191;
  yield _192;
  yield _193;
  yield _194;
  yield _195;
  yield _196;
  yield _197;
  yield _198;
  yield _199;
  yield _200;
  yield _201;
  yield _202;
  yield _203;
  yield _204;
  yield _205;
  yield _206;
  yield _207;
  yield _208;
  yield _209;
  yield _210;
  yield _211;
  yield _212;
  yield _213;
  yield _214;
  yield _215;
  yield _216;
  yield _217;
  yield _218;
  yield _219;
  yield _220;
  yield _221;
  yield _222;
  yield _223;
  yield _224;
  yield _225;
  yield _226;
  yield _227;
  yield _228;
  yield _229;
  yield _230;
  yield _231;
  yield _232;
  yield _233;
  yield _234;
  yield _235;
  yield _236;
  yield _237;
  yield _238;
  yield _239;
  yield _240;
  yield _241;
  yield _242;
  yield _243;
  yield _244;
  yield _245;
  yield _246;
  yield _247;
  yield _248;
  yield _249;
  yield _250;
  yield _251;
  yield _252;
  yield _253;
  yield _254;
  yield _255;
  yield _256;
  yield _257;
  yield _258;
  yield _259;
  yield _260;
  yield _261;
  yield _262;
  yield _263;
  yield _264;
  yield _265;
  yield _266;
  yield _267;
  yield _268;
  yield _269;
  yield _270;
  yield _271;
  yield _272;
  yield _273;
  yield _274;
  yield _275;
  yield _276;
  yield _277;
  yield _278;
  yield _279;
  yield _280;
  yield _281;
  yield _282;
  yield _283;
  yield _284;
  yield _285;
  yield _286;
  yield _287;
  yield _288;
  yield _289;
  yield _290;
  yield _291;
  yield _292;
  yield _293;
  yield _294;
  yield _295;
  yield _296;
  yield _297;
  yield _298;
  yield _299;
  yield _300;
}


function foo() {
    var res = 0;
    for (var v of gen()) {
        res += v;
    }
    return res;
}

print(foo());
// CHECK: 45451
