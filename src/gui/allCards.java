package gui;

import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/**
 * A class to contain all the cards in the Shandalar game.
 * The name is associated with a CodePair which stores the
 * code for the Duel deck as an int and the code for the Game
 * deck as a String.
 *@author Ryan Russell
 */
public class allCards {

    private final Map<String, CodePair> cardsByName;
    private final Map<String, String> namesByGameCode;
    private final Map<Integer, String> namesByDuelCode;
    private final Map<Integer, String> gameCodesByDuelCode;
    private final Map<String, Integer> duelCodesByGameCode;

    /**
     * When an allCards object is created we associate all
     * the values.
     */
    public allCards() {
        Map<String, CodePair> cards = new LinkedHashMap<>();
        cards.put("Swamp", new CodePair(239, "0000"));
        cards.put("Island", new CodePair(126, "0100"));
        cards.put("Forest", new CodePair(91, "0200"));
        cards.put("Mountain", new CodePair(164, "0300"));
        cards.put("Plains", new CodePair(188, "0400"));
        cards.put("Badlands", new CodePair(9, "0500"));
        cards.put("Bayou", new CodePair(12, "0600"));
        cards.put("Plateau", new CodePair(189, "0700"));
        cards.put("Savannah", new CodePair(212, "0800"));
        cards.put("Scrubland", new CodePair(216, "0900"));
        cards.put("Taiga", new CodePair(241, "0A00"));
        cards.put("Trop.Island", new CodePair(252, "0B00"));
        cards.put("Tundra", new CodePair(254, "0C00"));
        cards.put("Undergd.Sea", new CodePair(258, "0D00"));
        cards.put("Vol.Island", new CodePair(266, "0E00"));
        cards.put("Mesa Pegasus", new CodePair(161, "0F00"));
        cards.put("BenalishHero", new CodePair(13, "1000"));
        cards.put("Sav.Lions", new CodePair(213, "1100"));
        cards.put("N.Paladin", new CodePair(175, "1200"));
        cards.put("Serra Angel", new CodePair(221, "1300"));
        cards.put("V.Bodyguard", new CodePair(264, "1400"));
        cards.put("Samite Healr", new CodePair(211, "1500"));
        cards.put("PearledUnicn", new CodePair(180, "1600"));
        cards.put("WallofSwords", new CodePair(273, "1700"));
        cards.put("WhiteKnight", new CodePair(283, "1800"));
        cards.put("Atog", new CodePair(477, "1900"));
        cards.put("Orcish Atty.", new CodePair(177, "1A00"));
        cards.put("Dwarven Wpn", new CodePair(493, "1B00"));
        cards.put("Hill Giant", new CodePair(110, "1C00"));
        cards.put("Earth Elem.", new CodePair(73, "1D00"));
        cards.put("DragonWhelp", new CodePair(67, "1E00"));
        cards.put("Fire Elem.", new CodePair(83, "1F00"));
        cards.put("GoblinBalln", new CodePair(101, "2000"));
        cards.put("GoblinRaidrs", new CodePair(163, "2100"));
        cards.put("Goblin King", new CodePair(102, "2200"));
        cards.put("GraniteGarg.", new CodePair(103, "2300"));
        cards.put("Gray Ogre", new CodePair(104, "2400"));
        cards.put("HurloonMintr", new CodePair(115, "2500"));
        cards.put("KeldonWarlrd", new CodePair(135, "2600"));
        cards.put("Kird Ape", new CodePair(437, "2700"));
        cards.put("Roc of Kher", new CodePair(206, "2800"));
        cards.put("Rock Hydra", new CodePair(207, "2900"));
        cards.put("Sedge Troll", new CodePair(219, "2A00"));
        cards.put("ShivanDragon", new CodePair(224, "2B00"));
        cards.put("Stone Giant", new CodePair(235, "2C00"));
        cards.put("Uthden Troll", new CodePair(261, "2D00"));
        cards.put("Wall of Fire", new CodePair(270, "2E00"));
        cards.put("WallofStone", new CodePair(272, "2F00"));
        cards.put("IronclawOrcs", new CodePair(124, "3000"));
        cards.put("Rukh Egg", new CodePair(452, "3100"));
        cards.put("Merfolk", new CodePair(160, "3200"));
        cards.put("LordAtlantis", new CodePair(150, "3300"));
        cards.put("AirElemental", new CodePair(0, "3400"));
        cards.put("Sea Serpent", new CodePair(218, "3500"));
        cards.put("Clone", new CodePair(39, "3600"));
        cards.put("Wall of Air", new CodePair(267, "3700"));
        cards.put("Wall of H2O", new CodePair(274, "3800"));
        cards.put("IslandFish", new CodePair(427, "3900"));
        cards.put("Mahamoti Djn", new CodePair(154, "3A00"));
        cards.put("PhantasmFrce", new CodePair(183, "3B00"));
        cards.put("PhntomMonstr", new CodePair(185, "3C00"));
        cards.put("PirateShip", new CodePair(186, "3D00"));
        cards.put("ProdigalSorc", new CodePair(193, "3E00"));
        cards.put("WaterElement", new CodePair(279, "3F00"));
        cards.put("ZephyrFalcon", new CodePair(859, "4000"));
        cards.put("GiantTortse", new CodePair(422, "4100"));
        cards.put("GuardBeast", new CodePair(423, "4200"));
        cards.put("V.Doppelgngr", new CodePair(263, "4300"));
        cards.put("ShanodnDryad", new CodePair(222, "4400"));
        cards.put("Wall of Wood", new CodePair(275, "4500"));
        cards.put("Grizzly Bear", new CodePair(106, "4600"));
        cards.put("Fungusaur", new CodePair(94, "4700"));
        cards.put("War Mammoth", new CodePair(277, "4800"));
        cards.put("Giant Spider", new CodePair(98, "4900"));
        cards.put("Craw Wurm", new CodePair(49, "4A00"));
        cards.put("ElvishArchrs", new CodePair(76, "4B00"));
        cards.put("ForceONature", new CodePair(89, "4C00"));
        cards.put("IronrootTflk", new CodePair(125, "4D00"));
        cards.put("LlanowarElvs", new CodePair(149, "4E00"));
        cards.put("ScrybSprites", new CodePair(217, "4F00"));
        cards.put("TimberWolves", new CodePair(247, "5000"));
        cards.put("Wall/Bramble", new CodePair(269, "5100"));
        cards.put("Enchantress", new CodePair(262, "5200"));
        cards.put("DurkwdBoars", new CodePair(618, "5300"));
        cards.put("Drudge Skel.", new CodePair(70, "5400"));
        cards.put("ScatheZombie", new CodePair(214, "5500"));
        cards.put("ZombieMaster", new CodePair(291, "5600"));
        cards.put("Erg Raiders", new CodePair(415, "5700"));
        cards.put("Carrion Ants", new CodePair(589, "5800"));
        cards.put("Bog Wraith", new CodePair(24, "5900"));
        cards.put("Frozen Shade", new CodePair(93, "5A00"));
        cards.put("Nightmare", new CodePair(174, "5B00"));
        cards.put("RoyalAssasin", new CodePair(209, "5C00"));
        cards.put("SorceressQn", new CodePair(460, "5D00"));
        cards.put("WallofBone", new CodePair(268, "5E00"));
        cards.put("WillOtheWisp", new CodePair(286, "5F00"));
        cards.put("HypnoticSpec", new CodePair(117, "6000"));
        cards.put("Vampire Bats", new CodePair(835, "6100"));
        cards.put("Bad Moon", new CodePair(8, "6200"));
        cards.put("Cursed Land", new CodePair(53, "6300"));
        cards.put("EvilPresence", new CodePair(77, "6400"));
        cards.put("UnholyStrnth", new CodePair(259, "6500"));
        cards.put("Weakness", new CodePair(280, "6600"));
        cards.put("WarpArtifact", new CodePair(278, "6700"));
        cards.put("Unstable Mn.", new CodePair(462, "6800"));
        cards.put("Phantasmal", new CodePair(184, "6900"));
        cards.put("CopyArtifact", new CodePair(47, "6A00"));
        cards.put("Flight", new CodePair(87, "6B00"));
        cards.put("PsychicVenom", new CodePair(195, "6C00"));
        cards.put("CreatureBond", new CodePair(50, "6D00"));
        cards.put("Animate Art.", new CodePair(2, "6E00"));
        cards.put("Feedback", new CodePair(82, "6F00"));
        cards.put("Lifetap", new CodePair(144, "7000"));
        cards.put("Wild Growth", new CodePair(285, "7100"));
        cards.put("Regeneration", new CodePair(201, "7200"));
        cards.put("Web", new CodePair(281, "7300"));
        cards.put("Wanderlust", new CodePair(276, "7400"));
        cards.put("InstillEnrgy", new CodePair(121, "7500"));
        cards.put("Lifeforce", new CodePair(142, "7600"));
        cards.put("AspectofWolf", new CodePair(7, "7700"));
        cards.put("LivingArtfct", new CodePair(146, "7800"));
        cards.put("Earthbind", new CodePair(74, "7900"));
        cards.put("Burrowing", new CodePair(26, "7A00"));
        cards.put("Firebreath", new CodePair(85, "7B00"));
        cards.put("Manabarbs", new CodePair(158, "7C00"));
        cards.put("Oriflamme", new CodePair(178, "7D00"));
        cards.put("ManaFlare", new CodePair(155, "7E00"));
        cards.put("Holy Armor", new CodePair(111, "7F00"));
        cards.put("Castle", new CodePair(28, "8000"));
        cards.put("HolyStrength", new CodePair(112, "8100"));
        cards.put("Black Ward", new CodePair(19, "8200"));
        cards.put("Green Ward", new CodePair(105, "8300"));
        cards.put("Blue Ward", new CodePair(23, "8400"));
        cards.put("Red Ward", new CodePair(200, "8500"));
        cards.put("White Ward", new CodePair(284, "8600"));
        cards.put("Art Ward", new CodePair(473, "8700"));
        cards.put("Karma", new CodePair(134, "8800"));
        cards.put("Farmstead", new CodePair(79, "8900"));
        cards.put("COP White", new CodePair(37, "8A00"));
        cards.put("COP Black", new CodePair(33, "8B00"));
        cards.put("COP Blue", new CodePair(34, "8C00"));
        cards.put("COP Red", new CodePair(36, "8D00"));
        cards.put("COP Green", new CodePair(35, "8E00"));
        cards.put("Crusade", new CodePair(51, "8F00"));
        cards.put("BlazeOfGlory", new CodePair(20, "9000"));
        cards.put("Blessing", new CodePair(21, "9100"));
        cards.put("Conversion", new CodePair(45, "9200"));
        cards.put("SpiritLink", new CodePair(794, "9300"));
        cards.put("DivineTrans", new CodePair(616, "9400"));
        cards.put("Wrath of God", new CodePair(290, "9500"));
        cards.put("Armageddon", new CodePair(6, "9600"));
        cards.put("Resurrection", new CodePair(203, "9700"));
        cards.put("Raise Dead", new CodePair(198, "9800"));
        cards.put("Drain Life", new CodePair(68, "9900"));
        cards.put("Reconstruct", new CodePair(521, "9A00"));
        cards.put("Braingeyser", new CodePair(25, "9B00"));
        cards.put("Disintegrate", new CodePair(65, "9C00"));
        cards.put("Stone Rain", new CodePair(236, "9D00"));
        cards.put("EarthQuake", new CodePair(75, "9E00"));
        cards.put("Fireball", new CodePair(84, "9F00"));
        cards.put("Flashfires", new CodePair(86, "A000"));
        cards.put("Shatterstorm", new CodePair(526, "A100"));
        cards.put("DesertTwistr", new CodePair(409, "A200"));
        cards.put("Hurricane", new CodePair(116, "A300"));
        cards.put("Tranquility", new CodePair(251, "A400"));
        cards.put("StreamofLife", new CodePair(237, "A500"));
        cards.put("Ice Storm", new CodePair(118, "A600"));
        cards.put("Regrowth", new CodePair(202, "A700"));
        cards.put("Tsunami", new CodePair(253, "A800"));
        cards.put("Basalt Mono", new CodePair(11, "A900"));
        cards.put("Conservator", new CodePair(42, "AA00"));
        cards.put("GntletOMight", new CodePair(96, "AB00"));
        cards.put("Iron Star", new CodePair(123, "AC00"));
        cards.put("Ivory Cup", new CodePair(128, "AD00"));
        cards.put("Crystal Rod", new CodePair(52, "AE00"));
        cards.put("AnkhofMishra", new CodePair(5, "AF00"));
        cards.put("ArmageddnClk", new CodePair(471, "B000"));
        cards.put("Dingus Egg", new CodePair(63, "B100"));
        cards.put("Ebony Horse", new CodePair(412, "B200"));
        cards.put("JadeMonolith", new CodePair(129, "B300"));
        cards.put("JandorsSddle", new CodePair(430, "B400"));
        cards.put("JayemdaeTome", new CodePair(131, "B500"));
        cards.put("Mana Vault", new CodePair(157, "B600"));
        cards.put("Meekstone", new CodePair(159, "B700"));
        cards.put("Onulet", new CodePair(512, "B800"));
        cards.put("RocketLaunch", new CodePair(523, "B900"));
        cards.put("RodofRuin", new CodePair(208, "BA00"));
        cards.put("Sol Ring", new CodePair(230, "BB00"));
        cards.put("Soul Net", new CodePair(231, "BC00"));
        cards.put("ThroneofBone", new CodePair(246, "BD00"));
        cards.put("WoodenSphere", new CodePair(288, "BE00"));
        cards.put("Winter Orb", new CodePair(287, "BF00"));
        cards.put("FlyingCarpet", new CodePair(419, "C000"));
        cards.put("HelmofChatz", new CodePair(109, "C100"));
        cards.put("HowlingMine", new CodePair(114, "C200"));
        cards.put("Mightstone", new CodePair(506, "C300"));
        cards.put("SunglassUrza", new CodePair(238, "C400"));
        cards.put("Brass Man", new CodePair(930, "C500"));
        cards.put("D.Scimitar", new CodePair(931, "C600"));
        cards.put("DragonEngine", new CodePair(492, "C700"));
        cards.put("ClockworkBst", new CodePair(38, "C800"));
        cards.put("Living Wall", new CodePair(148, "C900"));
        cards.put("ObsianusGolm", new CodePair(176, "CA00"));
        cards.put("Ornithopter", new CodePair(514, "CB00"));
        cards.put("Disenchant", new CodePair(64, "CC00"));
        cards.put("Guard.Angel", new CodePair(107, "CD00"));
        cards.put("Death Ward", new CodePair(57, "CE00"));
        cards.put("Righteousnss", new CodePair(205, "CF00"));
        cards.put("SwordToPlow", new CodePair(240, "D000"));
        cards.put("HealingSalve", new CodePair(108, "D100"));
        cards.put("AlabasterPot", new CodePair(560, "D200"));
        cards.put("PsionicBlast", new CodePair(194, "D300"));
        cards.put("Hurkyl's Rcl", new CodePair(502, "D400"));
        cards.put("Jump", new CodePair(133, "D500"));
        cards.put("Unsummon", new CodePair(260, "D600"));
        cards.put("AncestralRcl", new CodePair(1, "D700"));
        cards.put("Mana Short", new CodePair(156, "D800"));
        cards.put("Howl/Beyond", new CodePair(113, "D900"));
        cards.put("Terror", new CodePair(242, "DA00"));
        cards.put("Shatter", new CodePair(223, "DB00"));
        cards.put("Tunnel", new CodePair(255, "DC00"));
        cards.put("Lightning", new CodePair(145, "DD00"));
        cards.put("Inferno", new CodePair(337, "DE00"));
        cards.put("Berserk", new CodePair(14, "DF00"));
        cards.put("Giant Growth", new CodePair(97, "E000"));
        cards.put("Simulacrum", new CodePair(225, "E100"));
        cards.put("Purelace", new CodePair(196, "E200"));
        cards.put("RagingRiver", new CodePair(197, "E300"));
        cards.put("BlueBlast", new CodePair(22, "E400"));
        cards.put("Counterspell", new CodePair(48, "E500"));
        cards.put("Spell Blast", new CodePair(232, "E600"));
        cards.put("Deathlace", new CodePair(59, "E700"));
        cards.put("Sacrifice", new CodePair(210, "E800"));
        cards.put("Dark Ritual", new CodePair(55, "E900"));
        cards.put("Chaoslace", new CodePair(32, "EA00"));
        cards.put("RedBlast", new CodePair(199, "EB00"));
        cards.put("Lifelace", new CodePair(143, "EC00"));
        cards.put("Thoughtlace", new CodePair(245, "ED00"));
        cards.put("Mox Emerald", new CodePair(165, "EE00"));
        cards.put("Mox Jet", new CodePair(166, "EF00"));
        cards.put("Mox Pearl", new CodePair(167, "F000"));
        cards.put("Mox Ruby", new CodePair(168, "F100"));
        cards.put("Mox Sapphire", new CodePair(169, "F200"));
        cards.put("Black Lotus", new CodePair(17, "F300"));
        cards.put("AladdinsRing", new CodePair(396, "F400"));
        cards.put("Celest.Prism", new CodePair(29, "F500"));
        cards.put("CopperTablet", new CodePair(46, "F600"));
        cards.put("DisruptScept", new CodePair(66, "F700"));
        cards.put("IcyManipultr", new CodePair(119, "F800"));
        cards.put("Consecrate", new CodePair(41, "F900"));
        cards.put("Lance", new CodePair(138, "FA00"));
        cards.put("ReverseDamge", new CodePair(204, "FB00"));
        cards.put("JandorsRing", new CodePair(429, "FC00"));
        cards.put("Jeweled Bird", new CodePair(431, "FD00"));
        cards.put("Jade Statue", new CodePair(130, "FE00"));
        cards.put("RingOfMaruf", new CodePair(451, "FF00"));
        cards.put("ErhnamDjn", new CodePair(416, "0001"));
        cards.put("CityofBrass", new CodePair(403, "0101"));
        cards.put("Desert", new CodePair(407, "0201"));
        cards.put("ArmyofAllah", new CodePair(397, "0301"));
        cards.put("Jihad", new CodePair(432, "0401"));
        cards.put("KingSuleiman", new CodePair(436, "0501"));
        cards.put("Moorish Cav.", new CodePair(443, "0601"));
        cards.put("Piety", new CodePair(448, "0701"));
        cards.put("Pyramids", new CodePair(449, "0801"));
        cards.put("Blacksmith", new CodePair(450, "0901"));
        cards.put("WarElephant", new CodePair(463, "0A01"));
        cards.put("Dandan", new CodePair(406, "0B01"));
        cards.put("FishliverOil", new CodePair(418, "0C01"));
        cards.put("FlyingMen", new CodePair(420, "0D01"));
        cards.put("MerchantShip", new CodePair(440, "0E01"));
        cards.put("SerendibDjn.", new CodePair(455, "0F01"));
        cards.put("SerendibEfr.", new CodePair(456, "1001"));
        cards.put("El-Hajjaj", new CodePair(413, "1101"));
        cards.put("HasranOgress", new CodePair(424, "1201"));
        cards.put("Junun Efreet", new CodePair(433, "1301"));
        cards.put("Juzam Djinn", new CodePair(434, "1401"));
        cards.put("Khabal Ghoul", new CodePair(435, "1501"));
        cards.put("Stone Devils", new CodePair(461, "1601"));
        cards.put("Ali Baba", new CodePair(394, "1701"));
        cards.put("AliFromCairo", new CodePair(395, "1801"));
        cards.put("Bird Maiden", new CodePair(399, "1901"));
        cards.put("Mijae Djinn", new CodePair(442, "1A01"));
        cards.put("Ghazban Ogre", new CodePair(421, "1B01"));
        cards.put("Sandstorm", new CodePair(454, "1C01"));
        cards.put("SandalsAbdal", new CodePair(453, "1D01"));
        cards.put("Plague Rats", new CodePair(187, "1E01"));
        cards.put("CuombajWitch", new CodePair(404, "1F01"));
        cards.put("BlackKnight", new CodePair(16, "2001"));
        cards.put("Camel", new CodePair(401, "2101"));
        cards.put("Juggernaut", new CodePair(132, "2201"));
        cards.put("Naf's Asp", new CodePair(444, "2301"));
        cards.put("Wyluli Wolf", new CodePair(464, "2401"));
        cards.put("Hurr Jackal", new CodePair(425, "2501"));
        cards.put("Invisibility", new CodePair(122, "2601"));
        cards.put("Fear", new CodePair(81, "2701"));
        cards.put("DwarvenDTeam", new CodePair(71, "2801"));
        cards.put("DwrvnWarrior", new CodePair(72, "2901"));
        cards.put("BirdOParadse", new CodePair(15, "2A01"));
        cards.put("GiantBadger", new CodePair(898, "2B01"));
        cards.put("Nettling Imp", new CodePair(172, "2C01"));
        cards.put("SengirVampre", new CodePair(220, "2D01"));
        cards.put("LordofthePit", new CodePair(151, "2E01"));
        cards.put("NetherShadow", new CodePair(171, "2F01"));
        cards.put("NevinyrrlDsk", new CodePair(173, "3001"));
        cards.put("Paralyze", new CodePair(179, "3101"));
        cards.put("AnimateDead", new CodePair(3, "3201"));
        cards.put("DemonicTutor", new CodePair(62, "3301"));
        cards.put("MindTwist", new CodePair(162, "3401"));
        cards.put("DiamondVally", new CodePair(410, "3501"));
        cards.put("IsleOfWakWak", new CodePair(428, "3601"));
        cards.put("Pestilence", new CodePair(182, "3701"));
        cards.put("The Hive", new CodePair(243, "3801"));
        cards.put("Forcefield", new CodePair(90, "3901"));
        cards.put("Power Leak", new CodePair(190, "3A01"));
        cards.put("DrainPower", new CodePair(69, "3B01"));
        cards.put("Twiddle", new CodePair(256, "3C01"));
        cards.put("2HeadedGiant", new CodePair(257, "3D01"));
        cards.put("Stasis", new CodePair(233, "3E01"));
        cards.put("Scav.Ghoul", new CodePair(215, "3F01"));
        cards.put("Sinkhole", new CodePair(226, "4001"));
        cards.put("MagneticMtn", new CodePair(439, "4101"));
        cards.put("Power Surge", new CodePair(192, "4201"));
        cards.put("Smoke", new CodePair(229, "4301"));
        cards.put("WheelFortune", new CodePair(282, "4401"));
        cards.put("Channel", new CodePair(30, "4501"));
        cards.put("Fastbond", new CodePair(80, "4601"));
        cards.put("Ley Druid", new CodePair(139, "4701"));
        cards.put("Lure", new CodePair(152, "4801"));
        cards.put("ThicketBslsk", new CodePair(244, "4901"));
        cards.put("Cockatrice", new CodePair(40, "4A01"));
        cards.put("WallofIce", new CodePair(271, "4B01"));
        cards.put("Magical Hack", new CodePair(153, "4C01"));
        cards.put("SleightOMind", new CodePair(228, "4D01"));
        cards.put("Black Vise", new CodePair(18, "4E01"));
        cards.put("Ivory Tower", new CodePair(503, "4F01"));
        cards.put("The Rack", new CodePair(535, "5001"));
        cards.put("ContractBelo", new CodePair(43, "5101"));
        cards.put("TimeTwister", new CodePair(250, "5201"));
        cards.put("AladdinsLamp", new CodePair(393, "5301"));
        cards.put("Millstone", new CodePair(507, "5401"));
        cards.put("BazaarOBaghd", new CodePair(398, "5501"));
        cards.put("LibraryOAlex", new CodePair(438, "5601"));
        cards.put("Sindbad", new CodePair(458, "5701"));
        cards.put("SingingTree", new CodePair(459, "5801"));
        cards.put("GoblnArtisns", new CodePair(498, "5901"));
        cards.put("MishraFactry", new CodePair(508, "5A01"));
        cards.put("MishraWrkshp", new CodePair(510, "5B01"));
        cards.put("Strip Mine", new CodePair(528, "5C01"));
        cards.put("Urza's Mine", new CodePair(541, "5D01"));
        cards.put("Urza'sPlant", new CodePair(543, "5E01"));
        cards.put("Urza'sTower", new CodePair(544, "5F01"));
        cards.put("ArgivianArch", new CodePair(467, "6001"));
        cards.put("ArgivianSmth", new CodePair(468, "6101"));
        cards.put("COPArtifacts", new CodePair(481, "6201"));
        cards.put("DampingField", new CodePair(489, "6301"));
        cards.put("MartyrOKorls", new CodePair(505, "6401"));
        cards.put("RvrsPolarity", new CodePair(522, "6501"));
        cards.put("DrafnaRstore", new CodePair(491, "6601"));
        cards.put("EnergyFlux", new CodePair(494, "6701"));
        cards.put("SageOfLatNam", new CodePair(524, "6801"));
        cards.put("TransmuteArt", new CodePair(537, "6901"));
        cards.put("Afct.Possess", new CodePair(516, "6A01"));
        cards.put("GatePhyrexia", new CodePair(497, "6B01"));
        cards.put("HauntingWind", new CodePair(501, "6C01"));
        cards.put("PhyrxGremlin", new CodePair(515, "6D01"));
        cards.put("PriestYwgmth", new CodePair(549, "6E01"));
        cards.put("XenicPolterg", new CodePair(547, "6F01"));
        cards.put("YwgmthDemon", new CodePair(548, "7001"));
        cards.put("ArtifactBlst", new CodePair(472, "7101"));
        cards.put("Detonate", new CodePair(490, "7201"));
        cards.put("OrcMechanic", new CodePair(513, "7301"));
        cards.put("ArgothnPixie", new CodePair(469, "7401"));
        cards.put("ArgothnTrflk", new CodePair(470, "7501"));
        cards.put("CitanulDruid", new CodePair(482, "7601"));
        cards.put("Crumble", new CodePair(487, "7701"));
        cards.put("GaeasAvenger", new CodePair(496, "7801"));
        cards.put("PowerLeech", new CodePair(518, "7901"));
        cards.put("AmuletOKroog", new CodePair(466, "7A01"));
        cards.put("AshnodsAltar", new CodePair(474, "7B01"));
        cards.put("AshnodsBtlgr", new CodePair(475, "7C01"));
        cards.put("AshnodsTrans", new CodePair(476, "7D01"));
        cards.put("BatteringRam", new CodePair(478, "7E01"));
        cards.put("CandlOTawnos", new CodePair(480, "7F01"));
        cards.put("ClayStatue", new CodePair(483, "8001"));
        cards.put("ClockworkAvn", new CodePair(484, "8101"));
        cards.put("ColosusOSard", new CodePair(485, "8201"));
        cards.put("Coral Helm", new CodePair(486, "8301"));
        cards.put("Cursed Rack", new CodePair(488, "8401"));
        cards.put("Feldons Cane", new CodePair(495, "8501"));
        cards.put("GrapeshotCpt", new CodePair(500, "8601"));
        cards.put("Jalum Tome", new CodePair(504, "8701"));
        cards.put("MishraWarMch", new CodePair(509, "8801"));
        cards.put("ObeliskOUndo", new CodePair(511, "8901"));
        cards.put("Primal Clay", new CodePair(519, "8A01"));
        cards.put("Rakalite", new CodePair(520, "8B01"));
        cards.put("StaffOfZegon", new CodePair(527, "8C01"));
        cards.put("Su-Chi", new CodePair(529, "8D01"));
        cards.put("TabltOEpityr", new CodePair(530, "8E01"));
        cards.put("TawnosCoffin", new CodePair(531, "8F01"));
        cards.put("TawnosWeapon", new CodePair(533, "9001"));
        cards.put("Triskelion", new CodePair(538, "9101"));
        cards.put("UrzasAvenger", new CodePair(539, "9201"));
        cards.put("UrzasChalice", new CodePair(540, "9301"));
        cards.put("UrzasMiter", new CodePair(542, "9401"));
        cards.put("WallofSpears", new CodePair(545, "9501"));
        cards.put("Weakstone", new CodePair(546, "9601"));
        cards.put("YotnSoldier", new CodePair(550, "9701"));
        cards.put("Time Walk", new CodePair(249, "9801"));
        cards.put("Time Vault", new CodePair(248, "9901"));
        cards.put("Balance", new CodePair(10, "9A01"));
        cards.put("CyclopeanTmb", new CodePair(54, "9B01"));
        cards.put("EyeForAnEye", new CodePair(417, "9C01"));
        cards.put("IslandSanct.", new CodePair(127, "9D01"));
        cards.put("PersonalInc.", new CodePair(181, "9E01"));
        cards.put("ControlMagic", new CodePair(44, "9F01"));
        cards.put("StealArtifct", new CodePair(234, "A001"));
        cards.put("Siren's Call", new CodePair(227, "A101"));
        cards.put("VolcanicErpt", new CodePair(265, "A201"));
        cards.put("Darkpact", new CodePair(56, "A301"));
        cards.put("DemonicHorde", new CodePair(61, "A401"));
        cards.put("DemonicAttny", new CodePair(60, "A501"));
        cards.put("Fork", new CodePair(92, "A601"));
        cards.put("GaeasLiege", new CodePair(95, "A701"));
        cards.put("Kudzu", new CodePair(137, "A801"));
        cards.put("LivingLands", new CodePair(147, "A901"));
        cards.put("Kormus Bell", new CodePair(136, "AA01"));
        cards.put("NaturalSelec", new CodePair(170, "AB01"));
        cards.put("BottleOSulmn", new CodePair(400, "AC01"));
        cards.put("GlassesOUrza", new CodePair(99, "AD01"));
        cards.put("LibraryOLeng", new CodePair(140, "AE01"));
        cards.put("Lich", new CodePair(141, "AF01"));
        cards.put("ElephGrvyard", new CodePair(414, "B001"));
        cards.put("OldManoftheC", new CodePair(446, "B101"));
        cards.put("Oubliette", new CodePair(447, "B201"));
        cards.put("DesertNomads", new CodePair(408, "B301"));
        cards.put("YdwenEfreet", new CodePair(465, "B401"));
        cards.put("Cyclone", new CodePair(405, "B501"));
        cards.put("DropofHoney", new CodePair(411, "B601"));
        cards.put("IfhBiffEfrt", new CodePair(426, "B701"));
        cards.put("Abu Jafar", new CodePair(391, "B801"));
        cards.put("Aladdin", new CodePair(392, "B901"));
        cards.put("AnimateWall", new CodePair(4, "BA01"));
        cards.put("CallfmGrave", new CodePair(860, "BB01"));
        cards.put("PrismaticDrg", new CodePair(861, "BC01"));
        cards.put("RainbowKnght", new CodePair(862, "BD01"));
        cards.put("PandorasBox", new CodePair(863, "BE01"));
        cards.put("Whimsy", new CodePair(864, "BF01"));
        cards.put("FaerieDragon", new CodePair(865, "C001"));
        cards.put("GoblinPolka", new CodePair(866, "C101"));
        cards.put("PowerStruggl", new CodePair(867, "C201"));
        cards.put("Aswan Jaguar", new CodePair(868, "C301"));
        cards.put("OrcCatapult", new CodePair(869, "C401"));
        cards.put("Gem Bazaar", new CodePair(870, "C501"));
        cards.put("NecropofAzar", new CodePair(871, "C601"));
        cards.put("Arena", new CodePair(899, "C701"));
        cards.put("Mana Crypt", new CodePair(894, "C801"));
        cards.put("NalathniDrgn", new CodePair(895, "C901"));
        cards.put("SewerOEstark", new CodePair(896, "CA01"));
        cards.put("WindseekCntr", new CodePair(897, "CB01"));
        cards.put("Bog Imp", new CodePair(336, "CC01"));
        cards.put("Carniv.Plant", new CodePair(305, "CD01"));
        cards.put("DiabolicMach", new CodePair(376, "CE01"));
        cards.put("Ghost Ship", new CodePair(326, "CF01"));
        cards.put("GiantStrngth", new CodePair(649, "D001"));
        cards.put("Immolation", new CodePair(679, "D101"));
        cards.put("Killer Bees", new CodePair(698, "D201"));
        cards.put("Land Leeches", new CodePair(339, "D301"));
        cards.put("Lost Soul", new CodePair(719, "D401"));
        cards.put("Pikeman", new CodePair(942, "D501"));
        cards.put("Seeker", new CodePair(781, "D601"));
        cards.put("SegovianLev", new CodePair(782, "D701"));
        cards.put("SistersFlame", new CodePair(360, "D801"));
        cards.put("TundraWolves", new CodePair(826, "D901"));
        cards.put("UncleIstvan", new CodePair(382, "DA01"));
        cards.put("SunkenCity", new CodePair(364, "DB01"));
        cards.put("BlueBattery", new CodePair(584, "DC01"));
        cards.put("WhiteBattery", new CodePair(852, "DD01"));
        cards.put("BlackBattery", new CodePair(580, "DE01"));
        cards.put("RedBattery", new CodePair(765, "DF01"));
        cards.put("GreenBattery", new CodePair(662, "E001"));
        cards.put("ApprenticeWz", new CodePair(300, "E101"));
        cards.put("Abomination", new CodePair(551, "E201"));
        cards.put("Venom", new CodePair(383, "E301"));
        cards.put("CyclopMummy", new CodePair(606, "E401"));
        cards.put("CosmicHorror", new CodePair(600, "E501"));
        cards.put("AngryMob", new CodePair(293, "E601"));
        cards.put("Blood Lust", new CodePair(583, "E701"));
        cards.put("FortArea", new CodePair(642, "E801"));
        cards.put("AmrouKithkin", new CodePair(563, "E901"));
        cards.put("ElvenRiders", new CodePair(622, "EA01"));
        cards.put("Visions", new CodePair(837, "EB01"));
        cards.put("BallLightng", new CodePair(295, "EC01"));
        cards.put("PsionicEnt.", new CodePair(749, "ED01"));
        cards.put("Morale", new CodePair(347, "EE01"));
        cards.put("CavePeople", new CodePair(306, "EF01"));
        cards.put("Blight", new CodePair(582, "F001"));
        cards.put("TheBrute", new CodePair(813, "F101"));
        cards.put("EternalWar", new CodePair(628, "F201"));
        cards.put("PradeshGyp", new CodePair(745, "F301"));
        cards.put("Ashes2Ashes", new CodePair(294, "F401"));
        cards.put("Fissure", new CodePair(321, "F501"));
        cards.put("WindsOChange", new CodePair(854, "F601"));
        cards.put("WordOBinding", new CodePair(388, "F701"));
        cards.put("FellwarStone", new CodePair(318, "F801"));
        cards.put("ManaClash", new CodePair(343, "F901"));
        cards.put("MarshViper", new CodePair(939, "FA01"));
        cards.put("MindBomb", new CodePair(346, "FB01"));
        cards.put("TimeElementl", new CodePair(817, "FC01"));
        cards.put("UntamedWilds", new CodePair(831, "FD01"));
        cards.put("Backfire", new CodePair(575, "FE01"));
        cards.put("ElderLandWrm", new CodePair(620, "FF01"));
        cards.put("Erosion", new CodePair(314, "0002"));
        cards.put("Flood", new CodePair(935, "0102"));
        cards.put("Gaseous Form", new CodePair(645, "0202"));
        cards.put("GoblnRockSld", new CodePair(937, "0302"));
        cards.put("Greed", new CodePair(661, "0402"));
        cards.put("Kismet", new CodePair(699, "0502"));
        cards.put("Leviathan", new CodePair(340, "0602"));
        cards.put("Murk Dweller", new CodePair(371, "0702"));
        cards.put("Osai Vulture", new CodePair(736, "0802"));
        cards.put("Pyrotechnics", new CodePair(752, "0902"));
        cards.put("Relic Bind", new CodePair(768, "0A02"));
        cards.put("WhirlingDerv", new CodePair(851, "0B02"));
        cards.put("Winter Blast", new CodePair(855, "0C02"));
        cards.put("BrotherOFire", new CodePair(304, "0D02"));
        cards.put("Energy Tap", new CodePair(626, "0E02"));
        cards.put("Marsh Gas", new CodePair(365, "0F02"));
        cards.put("RadjanSpirit", new CodePair(756, "1002"));
        cards.put("SpiritShckle", new CodePair(795, "1102"));
        cards.put("Pit Scorpion", new CodePair(742, "1202"));
        cards.put("Brainwash", new CodePair(303, "1302"));
        cards.put("CrimsonMant", new CodePair(604, "1402"));
        cards.put("SylvnLibrary", new CodePair(803, "1502"));
        cards.put("Land Tax", new CodePair(710, "1602"));
        cards.put("Rag Man", new CodePair(943, "1702"));
        cards.put("Rebirth", new CodePair(763, "1802"));
        cards.put("TempestEfrt", new CodePair(810, "1902"));
        cards.put("Shapeshifter", new CodePair(525, "1A02"));
        cards.put("Fog", new CodePair(88, "1B02"));
        cards.put("Deathgrip", new CodePair(58, "1C02"));
        cards.put("Tawnos'sWand", new CodePair(532, "1D02"));
        cards.put("Wall of Dust", new CodePair(841, "1E02"));
        cards.put("BronzeTablet", new CodePair(479, "1F02"));
        cards.put("Tetravus", new CodePair(534, "2002"));
        cards.put("TitaniasSong", new CodePair(536, "2102"));
        cards.put("Power Sink", new CodePair(191, "2202"));
        cards.put("Gloom", new CodePair(100, "2302"));
        cards.put("Oasis", new CodePair(445, "2402"));
        cards.put("CrimsonKobolds", new CodePair(603, "2502"));
        cards.put("CrookKobolds", new CodePair(605, "2602"));
        cards.put("KoboldsOKher", new CodePair(704, "2702"));
        cards.put("Mountain Yeti", new CodePair(730, "2802"));
        cards.put("Raging Bull", new CodePair(757, "2902"));
        cards.put("Wall Of Earth", new CodePair(842, "2A02"));
        cards.put("Wall Of Heat", new CodePair(843, "2B02"));
        cards.put("Goblin Hero", new CodePair(330, "2C02"));
        cards.put("Azure Drake", new CodePair(573, "2D02"));
        cards.put("Devouring Deep", new CodePair(612, "2E02"));
        cards.put("Barbary Apes", new CodePair(576, "2F02"));
        cards.put("Cat Warriors", new CodePair(590, "3002"));
        cards.put("Hornet Cobra", new CodePair(674, "3102"));
        cards.put("Moss Monster", new CodePair(728, "3202"));
        cards.put("HeadlessHMan", new CodePair(667, "3302"));
        cards.put("KeepersOFaith", new CodePair(696, "3402"));
        cards.put("RighteousAvers", new CodePair(774, "3502"));
        cards.put("Thunder Spirit", new CodePair(816, "3602"));
        cards.put("Wall Of Light", new CodePair(844, "3702"));
        cards.put("KnightsOThorn", new CodePair(338, "3802"));
        cards.put("Squire", new CodePair(948, "3902"));
        cards.put("Acid Rain", new CodePair(552, "3A02"));
        cards.put("Darkness", new CodePair(609, "3B02"));
        cards.put("Holy Day", new CodePair(672, "3C02"));
        cards.put("InfernalMedusa", new CodePair(683, "3D02"));
        cards.put("Lifeblood", new CodePair(715, "3E02"));
        cards.put("Walking Dead", new CodePair(839, "3F02"));
        cards.put("Boomerang", new CodePair(585, "4002"));
        cards.put("Cleanse", new CodePair(596, "4102"));
        cards.put("DAvenantArcher", new CodePair(607, "4202"));
        cards.put("DivineOffering", new CodePair(615, "4302"));
        cards.put("Exorcist", new CodePair(316, "4402"));
        cards.put("Fallen Angel", new CodePair(631, "4502"));
        cards.put("FountainOYouth", new CodePair(324, "4602"));
        cards.put("GhostsODamned", new CodePair(647, "4702"));
        cards.put("GoblinDigTeam", new CodePair(936, "4802"));
        cards.put("Great Defender", new CodePair(658, "4902"));
        cards.put("Hell Swarm", new CodePair(669, "4A02"));
        cards.put("Inquisition", new CodePair(374, "4B02"));
        cards.put("Jovial Evil", new CodePair(692, "4C02"));
        cards.put("MerfolkAssassn", new CodePair(940, "4D02"));
        cards.put("PeopleOWoods", new CodePair(323, "4E02"));
        cards.put("Relic Barrier", new CodePair(767, "4F02"));
        cards.put("Remove Soul", new CodePair(770, "5002"));
        cards.put("Riptide", new CodePair(355, "5102"));
        cards.put("Scavenger Folk", new CodePair(947, "5202"));
        cards.put("Shield Wall", new CodePair(786, "5302"));
        cards.put("Spinal Villain", new CodePair(793, "5402"));
        cards.put("Storm Seeker", new CodePair(798, "5502"));
        cards.put("The Drowned", new CodePair(370, "5602"));
        cards.put("Typhoon", new CodePair(827, "5702"));
        cards.put("WallOOppositn", new CodePair(845, "5802"));
        cards.put("Water Wurm", new CodePair(951, "5902"));
        cards.put("Alchors Tomb", new CodePair(561, "5A02"));
        cards.put("Angelic Voices", new CodePair(564, "5B02"));
        cards.put("Banshee", new CodePair(296, "5C02"));
        cards.put("BeastOBogardan", new CodePair(579, "5D02"));
        cards.put("Bog Rats", new CodePair(350, "5E02"));
        cards.put("Bone Flute", new CodePair(302, "5F02"));
        cards.put("ElvesODeepShad", new CodePair(313, "6002"));
        cards.put("Emerald Dfly", new CodePair(623, "6102"));
        cards.put("Eternal Flame", new CodePair(315, "6202"));
        cards.put("Fire Drake", new CodePair(320, "6302"));
        cards.put("Giant Turtle", new CodePair(650, "6402"));
        cards.put("GrterRealmOPrs", new CodePair(660, "6502"));
        cards.put("Hidden Path", new CodePair(335, "6602"));
        cards.put("Holy Light", new CodePair(375, "6702"));
        cards.put("IvoryGuardians", new CodePair(686, "6802"));
        cards.put("KoboldDSerge", new CodePair(701, "6902"));
        cards.put("KoboldOverlord", new CodePair(702, "6A02"));
        cards.put("KoboldTmaster", new CodePair(703, "6B02"));
        cards.put("Life Chisel", new CodePair(713, "6C02"));
        cards.put("Miracle Worker", new CodePair(941, "6D02"));
        cards.put("Mold Demon", new CodePair(727, "6E02"));
        cards.put("Pixie Queen", new CodePair(743, "6F02"));
        cards.put("Reset", new CodePair(771, "7002"));
        cards.put("Savaen Elves", new CodePair(944, "7102"));
        cards.put("SerpentGener", new CodePair(784, "7202"));
        cards.put("SpiritualSanct", new CodePair(796, "7302"));
        cards.put("Syphon Soul", new CodePair(805, "7402"));
        cards.put("WallOTombstone", new CodePair(848, "7502"));
        cards.put("War Barge", new CodePair(385, "7602"));
        cards.put("Witch Hunter", new CodePair(387, "7702"));
        cards.put("AkronLegnnaire", new CodePair(558, "7802"));
        cards.put("Amnesia", new CodePair(292, "7902"));
        cards.put("Barl's Cage", new CodePair(297, "7A02"));
        cards.put("Blood Moon", new CodePair(299, "7B02"));
        cards.put("Book Of Rass", new CodePair(353, "7C02"));
        cards.put("Coal Golem", new CodePair(308, "7D02"));
        cards.put("Elder Spawn", new CodePair(621, "7E02"));
        cards.put("Fire Sprites", new CodePair(635, "7F02"));
        cards.put("Force Spike", new CodePair(640, "8002"));
        cards.put("GoblinsOFlarg", new CodePair(332, "8102"));
        cards.put("HyperionBsmith", new CodePair(677, "8202"));
        cards.put("Martyr's Cry", new CodePair(345, "8302"));
        cards.put("Moat", new CodePair(726, "8402"));
        cards.put("Rabid Wombat", new CodePair(755, "8502"));
        cards.put("Tracker", new CodePair(380, "8602"));
        cards.put("WallOfWonder", new CodePair(850, "8702"));
        cards.put("Wormwood Tfolk", new CodePair(389, "8802"));
        cards.put("AssemblyWrkr", new CodePair(910, "8902"));
        cards.put("GiantWasp", new CodePair(885, "8A02"));
        cards.put("BottleDjinn", new CodePair(890, "8B02"));
        cards.put("SpawnofAzar", new CodePair(900, "8C02"));
        cards.put("XCyclopean", new CodePair(902, "8D02"));
        cards.put("Tetravite", new CodePair(891, "8E02"));
        cards.put("Rukh", new CodePair(892, "8F02"));
        cards.put("PoisonSnake", new CodePair(887, "9002"));
        cards.put("Dummy", new CodePair(916, "9102"));
        cards.put("Dummy", new CodePair(916, "9202"));
        cards.put("Dummy", new CodePair(916, "9302"));
        cards.put("DataCard", new CodePair(909, "9402"));
        cards.put("Damage", new CodePair(901, "9502"));
        cards.put("PowerUp", new CodePair(903, "9602"));
        cards.put("Unblockable", new CodePair(903, "9702"));
        cards.put("AddAbility", new CodePair(903, "9802"));
        cards.put("Sleighted", new CodePair(902, "9902"));
        cards.put("Hacked", new CodePair(902, "9A02"));
        cards.put("Stoning", new CodePair(903, "9B02"));
        cards.put("TakeAbility", new CodePair(903, "9C02"));
        cards.put("NoAttack", new CodePair(903, "9D02"));
        cards.put("DrawCard", new CodePair(904, "9E02"));
        cards.put("Hunting", new CodePair(905, "9F02"));
        cards.put("Polka", new CodePair(903, "A002"));
        cards.put("MarshGas", new CodePair(903, "A102"));
        cards.put("FogEffect", new CodePair(903, "A202"));
//cards.put("Channel", new CodePair(902,"A302")); //TODO - Why does this have 2 entries?
        cards.put("Generic", new CodePair(903, "A402"));
        cards.put("Asp Sting", new CodePair(902, "A502"));
        cards.put("NetherLink", new CodePair(902, "A602"));
        cards.put("Graveyard", new CodePair(902, "A702"));
        cards.put("Activation", new CodePair(906, "A802"));
        cards.put("PoltergeistFX", new CodePair(903, "A902"));
        cards.put("EbonyHorseFX", new CodePair(903, "AA02"));
        cards.put("TitaniasLeg", new CodePair(902, "AB02"));
        cards.put("DisintegrtFX", new CodePair(903, "AC02"));
        cards.put("SirensCallFX", new CodePair(903, "AD02"));
        cards.put("Damage Legacy", new CodePair(902, "AE02"));
        cards.put("AsteriskFX", new CodePair(903, "AF02"));
        cards.put("PiggyFX", new CodePair(903, "B002"));
        cards.put("TElementalFX", new CodePair(903, "B102"));
        cards.put("RukhEggFX", new CodePair(903, "B202"));
        cards.put("NettlingImpFX", new CodePair(903, "B302"));
        cards.put("ErhnamDjinnFX", new CodePair(903, "B402"));
        cards.put("DesertFX", new CodePair(903, "B502"));
        cards.put("PGremlinFX", new CodePair(903, "B602"));
        cards.put("XmorgrantFX", new CodePair(903, "B702"));
        cards.put("CTombFX", new CodePair(903, "B802"));
        cards.put("GuardianFX", new CodePair(903, "B902"));
        cards.put("IfhBiffFX", new CodePair(902, "BA02"));
        cards.put("SewerFX", new CodePair(903, "BB02"));
        cards.put("ControlFX", new CodePair(903, "BC02"));
        cards.put("LivingLandFX", new CodePair(903, "BD02"));
        cards.put("BlazeFX", new CodePair(903, "BE02"));
        cards.put("RiverFX", new CodePair(903, "BF02"));
        cards.put("BeastFX", new CodePair(903, "C002"));
        cards.put("TransmuteFX", new CodePair(903, "C102"));
        cards.put("Dummy", new CodePair(65535, "C202"));
        cards.put("Dummy", new CodePair(65535, "C302"));
        cards.put("Dummy", new CodePair(65535, "C402"));
        cards.put("Dummy", new CodePair(65535, "C502"));
        cards.put("Dummy", new CodePair(65535, "C602"));
        cards.put("Dummy", new CodePair(65535, "C702"));
        cards.put("Dummy", new CodePair(65535, "C802"));

        this.cardsByName = Map.copyOf(cards);
        this.namesByGameCode = buildNamesByGameCode(cards);
        this.namesByDuelCode = buildNamesByDuelCode(cards);
        this.gameCodesByDuelCode = buildGameCodesByDuelCode(cards);
        this.duelCodesByGameCode = buildDuelCodesByGameCode(cards);
    }

    /**
     * Takes a String representing the code of a card in a game deck
     * and returns that card's name as a String. Strips the gameCode
     * down to just the first 2 bytes and also takes care of some funny
     * business with cards in no decks or just deck 2.
     * <p>
     * The funny business is that if a card isn't in any decks or is only in deck 2 then
     * the 2nd byte in its code is set to a different value from the rest. This means if we
     * have a card like Flood that is normally 0102 it is changed to 0142 which stops it
     * being found. Actually this happens in other cases as well so rather than find them all
     * to take care of it we just set it back to 0 if it is 4. We should really check
     * what cases cause this and look for those cases but it looks like that would be more
     * trouble that it is worth. There is no needed to write in the 4 when saving as it
     * works fine without it however the game will add it back in when saving normally.
     * <p>
     * A new way of reading the cards (basically replacing the 4 with a 0 when we read)
     * has been added so this shouldn't be a problem at this stage anymore.
     * <p>
     * Returns null if the code is not in the map.
     *
     * @param gameCode A String representation of the hexadecimal game card code
     * @return String
     */
    public String gameToName(String gameCode) {
        return namesByGameCode.get(normalizeGameCode(gameCode));
    }

    /**
     * Takes an int representing the code of a card in a duel deck
     * and returns that card's name as a String.
     * <p>
     * Returns null if the code is not in the map.
     *
     * @param duelCode An int representation of the decimal duel card code
     * @return String
     */
    public String duelToName(int duelCode) {
        return namesByDuelCode.get(duelCode);
    }

    /**
     * Takes an int representing the code of a card in a duel deck
     * and returns that card's game code as a String.
     * <p>
     * Returns null if the code is not in the map.
     *
     * @param duelCode An int representation of the decimal duel card code
     * @return String
     */
    public String duelToGame(int duelCode) {
        return gameCodesByDuelCode.get(duelCode);
    }

    /**
     * Takes a String representing the code of a card stored in a game deck and returns
     * an int representation of the same card in a duel deck.
     * <p>
     * Returns -1 if the card is not in the map.
     *
     * @param gameCode The code of the card to convert
     * @return
     */
    public int gameToDuel(String gameCode) {
        return duelCodesByGameCode.getOrDefault(normalizeGameCode(gameCode), -1);
    }

    /**
     * Legacy map-like presence check retained for verification and older callers.
     *
     * @param name The card name to look up.
     * @return true if this map contains the card name.
     */
    public boolean containsKey(String name) {
        return cardsByName.containsKey(name);
    }

    private static Map<String, String> buildNamesByGameCode(Map<String, CodePair> cards) {
        Map<String, String> result = new HashMap<>();
        cards.forEach((name, codePair) -> result.put(codePair.gameCode(), name));
        return Map.copyOf(result);
    }

    private static Map<Integer, String> buildNamesByDuelCode(Map<String, CodePair> cards) {
        Map<Integer, String> result = new HashMap<>();
        cards.forEach((name, codePair) -> result.put(codePair.duelCode(), name));
        return Map.copyOf(result);
    }

    private static Map<Integer, String> buildGameCodesByDuelCode(Map<String, CodePair> cards) {
        Map<Integer, String> result = new HashMap<>();
        cards.values().forEach(codePair -> result.put(codePair.duelCode(), codePair.gameCode()));
        return Map.copyOf(result);
    }

    private static Map<String, Integer> buildDuelCodesByGameCode(Map<String, CodePair> cards) {
        Map<String, Integer> result = new HashMap<>();
        cards.values().forEach(codePair -> result.put(codePair.gameCode(), codePair.duelCode()));
        return Map.copyOf(result);
    }

    private static String normalizeGameCode(String gameCode) {
        if (gameCode == null || gameCode.length() < 4) {
            return gameCode;
        }

        String normalized = gameCode.substring(0, 4).toUpperCase(Locale.ROOT);

        if (normalized.charAt(2) == '4') {
            return normalized.substring(0, 2) + '0' + normalized.charAt(3);
        }

        return normalized;
    }

    private record CodePair(int duelCode, String gameCode) {
    }

}
