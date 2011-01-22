/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Illidari_Council
SD%Complete: 99
SDComment: Test event reseting and saving to DB (simplify some code..?)
SDCategory: Black Temple
EndScriptData */

#include "precompiled.h"
#include "def_black_temple.h"

//Speech'n'Sounds
#define SAY_GATH_SLAY           -1564085
#define SAY_GATH_SLAY_COMNT     -1564089
#define SAY_GATH_DEATH          -1564093
#define SAY_GATH_SPECIAL1       -1564077
#define SAY_GATH_SPECIAL2       -1564081

#define SAY_VERA_SLAY           -1564086
#define SAY_VERA_COMNT          -1564089
#define SAY_VERA_DEATH          -1564094
#define SAY_VERA_SPECIAL1       -1564078
#define SAY_VERA_SPECIAL2       -1564082

#define SAY_MALA_SLAY           -1564087
#define SAY_MALA_COMNT          -1564090
#define SAY_MALA_DEATH          -1564095
#define SAY_MALA_SPECIAL1       -1564079
#define SAY_MALA_SPECIAL2       -1564083

#define SAY_ZERE_SLAY           -1564088
#define SAY_ZERE_COMNT          -1564091
#define SAY_ZERE_DEATH          -1564096
#define SAY_ZERE_SPECIAL1       -1564080
#define SAY_ZERE_SPECIAL2       -1564084

struct CouncilYells
{
    int32 entry;
    uint32 timer;
};

static CouncilYells CouncilAggro[]=
{
    {-1564069, 5000},                                       // Gathios
    {-1564070, 5500},                                       // Veras
    {-1564071, 5000},                                       // Malande
    {-1564072, 1200000},                                    // Zerevor
};

// Need to get proper timers for this later
static CouncilYells CouncilEnrage[]=
{
    {-1564073, 2000},                                       // Gathios
    {-1564074, 6000},                                       // Veras
    {-1564075, 5000},                                       // Malande
    {-1564076, 1200000},                                    // Zerevor
};

#define SPELL_BERSERK 45078

// Gathios the Shatterer's spells
enum gathiosSpells
{
    SPELL_BLESS_PROTECTION     = 41450,
    SPELL_BLESS_SPELLWARD      = 41451,
    SPELL_CONSECRATION         = 41541,
    SPELL_HAMMER_OF_JUSTICE    = 41468,
    SPELL_SEAL_OF_COMMAND      = 41469,
    SPELL_SEAL_OF_BLOOD        = 41459,
    SPELL_GATHIOS_JUDGEMENT    = 41467,
    SPELL_CHROMATIC_AURA       = 41453,
    SPELL_DEVOTION_AURA        = 41452
};

// High Nethermancer Zerevor's spells
enum zerevorSpells
{
    SPELL_FLAMESTRIKE       = 41481,
    SPELL_BLIZZARD          = 41482,
    SPELL_ARCANE_BOLT       = 41483,
    SPELL_ARCANE_EXPLOSION  = 41524,
    SPELL_DAMPEN_MAGIC      = 41478
};

// Lady Malande's spells
enum malandeSpells
{
    SPELL_EMPOWERED_SMITE   = 41471,
    SPELL_CIRCLE_OF_HEALING = 41455,
    SPELL_REFLECTIVE_SHIELD = 41475,
    SPELL_DIVINE_WRATH      = 41472,
    SPELL_HEAL_VISUAL       = 24171
};

// Veras Darkshadow's spells
enum verasSpells
{
    SPELL_DEADLY_POISON = 41480,
    SPELL_VANISH          = 41476
    // Spell Envenom triggered by Deadly Poison in Aura::HandlePeriodicDamage
};

struct TRINITY_DLL_DECL mob_blood_elf_council_voice_triggerAI : public ScriptedAI
{
    mob_blood_elf_council_voice_triggerAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)me->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint64 m_council[4];

    uint32 m_enrageTimer;
    uint32 m_yellTimer;

    uint8 m_counter;                                      // Serves as the counter for both the aggro and enrage yells

    void Reset()
    {
        m_enrageTimer = 900000;                               // 15 minutes
        m_yellTimer = 500;

        m_counter = 0;
    }

    void AssignGUIDs()
    {
        m_council[0] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
        m_council[1] = pInstance->GetData64(DATA_VERASDARKSHADOW);
        m_council[2] = pInstance->GetData64(DATA_LADYMALANDE);
        m_council[3] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
    }

    void AttackStart(Unit *pWho){}

    void MoveInLineOfSight(Unit *pWho){}

    void UpdateAI(const uint32 diff)
    {
        if (pInstance->GetData(EVENT_ILLIDARICOUNCIL) != IN_PROGRESS)
            return;

        if (m_counter > 3)
            return;

        if (m_yellTimer < diff)
        {
            if (Unit *pMember = me->GetCreature(m_council[m_counter]))
            {
                DoScriptText(CouncilAggro[m_counter].entry, pMember);
                m_yellTimer = CouncilAggro[m_counter].timer;
            }

            m_counter += 1;
            if(m_counter > 3)
                m_counter = 0;                            // Reuse for Enrage Yells
        }
        else
            m_yellTimer -= diff;

        if (m_enrageTimer < diff)
        {
            if (Creature* pMember = pInstance->GetCreature(m_council[m_counter]))
            {
                pMember->CastSpell(pMember, SPELL_BERSERK, true);

                DoScriptText(CouncilEnrage[m_counter].entry, pMember);
                m_enrageTimer = CouncilEnrage[m_counter].timer;
            }

            m_counter += 1;
        }
        else
            m_enrageTimer -= diff;
    }
};

struct TRINITY_DLL_DECL mob_illidari_councilAI : public ScriptedAI
{
    mob_illidari_councilAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    uint64 m_council[4];

    void Reset()
    {
        pInstance->SetData(EVENT_ILLIDARICOUNCIL, NOT_STARTED);

        if(Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->AI()->EnterEvadeMode();

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetDisplayId(11686);
    }

    void StartEvent(Unit *pTarget)
    {
        if (pTarget->isAlive())
        {
            m_council[0] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
            m_council[1] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
            m_council[2] = pInstance->GetData64(DATA_LADYMALANDE);
            m_council[3] = pInstance->GetData64(DATA_VERASDARKSHADOW);

            // Start the event for the Voice Trigger
            if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
                ((mob_blood_elf_council_voice_triggerAI*)pTrigger->AI())->AssignGUIDs();

            for (uint8 i = 0; i < 4; ++i)
            {
                if (m_council[i])
                {
                    if (Unit *pMember = pInstance->GetCreature(m_council[i]))
                    {
                        if (pMember->isAlive())
                            ((Creature*)pMember)->AI()->AttackStart(pTarget);
                    }
                }
            }
            pInstance->SetData(EVENT_ILLIDARICOUNCIL, IN_PROGRESS);
        }
    }

    void JustDied(Unit *pVictim)
    {
        if (Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->Kill(pTrigger, false);

        pInstance->SetData(EVENT_ILLIDARICOUNCIL, DONE);
    }
};

struct TRINITY_DLL_DECL illidari_council_baseAI : public ScriptedAI
{
    illidari_council_baseAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        loadedGUIDs = false;
    }

    uint64 m_council[4];

    uint32 m_checkTimer;

    ScriptedInstance* pInstance;

    bool loadedGUIDs;

    void EnterCombat(Unit *pWho)
    {
        if (Creature *pController = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDARICOUNCIL)))
            ((mob_illidari_councilAI*)pController->AI())->StartEvent(pWho);

        DoZoneInCombat();

        if(me->GetEntry() == 22950)  // Zerevor
        {
            ClearCastQueue();
            ForceSpellCast(m_creature, SPELL_DAMPEN_MAGIC);
            AddSpellToCast(pWho, SPELL_ARCANE_BOLT);
            StartAutocast();
        }

        if (!loadedGUIDs)
        {
            m_council[0] = pInstance->GetData64(DATA_LADYMALANDE);
            m_council[1] = pInstance->GetData64(DATA_HIGHNETHERMANCERZEREVOR);
            m_council[2] = pInstance->GetData64(DATA_GATHIOSTHESHATTERER);
            m_council[3] = pInstance->GetData64(DATA_VERASDARKSHADOW);
            loadedGUIDs = true;
        }
    }

    void KilledUnit(Unit *pVictim)
    {
        switch (me->GetEntry())
        {
            case 22949: DoScriptText(SAY_GATH_SLAY, m_creature); break; // Gathios
            case 22950: DoScriptText(SAY_ZERE_SLAY, m_creature); break; // Zerevor
            case 22951: DoScriptText(SAY_MALA_SLAY, m_creature); break; // Melande
            case 22952: DoScriptText(SAY_VERA_SLAY, m_creature); break; // Veras
        }
    }

    void JustDied(Unit *pVictim)
    {
        switch (me->GetEntry())
        {
            case 22949: DoScriptText(SAY_GATH_DEATH, m_creature); break; // Gathios
            case 22950: DoScriptText(SAY_ZERE_DEATH, m_creature); break; // Zerevor
            case 22951: DoScriptText(SAY_MALA_DEATH, m_creature); break; // Melande
            case 22952: DoScriptText(SAY_VERA_DEATH, m_creature); break; // Veras
        }
        if (Creature *pCouncil = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDARICOUNCIL)))
        {
            if(pCouncil->isAlive())
                pCouncil->Kill(pCouncil, false);
        }
    }

    void EnterEvadeMode()
    {
        if(!pInstance)
            return;
        pInstance->SetData(EVENT_ILLIDARICOUNCIL, NOT_STARTED);

        if(Creature *pTrigger = pInstance->GetCreature(pInstance->GetData64(DATA_BLOOD_ELF_COUNCIL_VOICE)))
            pTrigger->AI()->EnterEvadeMode();
        ScriptedAI::EnterEvadeMode();
    }

    void SharedRule(uint32 &damage)
    {
        uint32 HP = 0;
        bool canKill = false;
        // get shared HP
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if(pUnit->GetHealth() == 0)
                    canKill = true;
                HP += pUnit->GetHealth();
                pUnit->LowerPlayerDamageReq(damage);
            }
        }
        // set shared HP
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if (pUnit->isAlive())
                {
                    if(HP)
                        pUnit->SetHealth(HP/4);
                }
            }
        }
        // if one dies, they die all
        if(!canKill)
            return;
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Creature *pUnit = pInstance->GetCreature(m_council[i]))
            {
                if(pUnit->isAlive())
                    pUnit->Kill(pUnit, false);
            }
        }
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if(done_by == m_creature)
            return;
        SharedRule(damage);
    }
};

// Gathios the Shatterer's AI
struct TRINITY_DLL_DECL boss_gathios_the_shattererAI : public illidari_council_baseAI
{
    boss_gathios_the_shattererAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_sealTimer;
    uint32 m_auraTimer;
    uint32 m_hammerTimer;
    uint32 m_blessingTimer;
    uint32 m_judgementTimer;
    uint32 m_consecrationTimer;

    uint32 m_checkTimer;

    void Reset()
    {
        m_consecrationTimer = urand(6000, 10000);
        m_hammerTimer = 10000;
        m_sealTimer = 1000;
        m_auraTimer = urand(3000, 30000);
        m_blessingTimer = urand(35000, 50000);
        m_judgementTimer = 16000;

        m_checkTimer = 1000;
    }

    Unit* SelectCouncil()
    {
        if(urand(0, 8)) // 8/9 chances to select Malande
        {
            if (Unit *pMelande = pInstance->GetCreature(m_council[0]))
            {
                if (pMelande->isAlive())
                    return pMelande;
            }
        }

        if (Unit *pCouncil = pInstance->GetCreature(m_council[urand(0,1)?1:3])) // else, select others, but never self
        {
            if (pCouncil->isAlive())
                return pCouncil;
        }

        return me;
    }

    void ApplyAura(uint32 m_spellId)        //should that be casted on ALL members? or just be applied as area aura within 30 yd??
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (Unit *pCouncil = pInstance->GetCreature(m_council[i]))
                pCouncil->CastSpell(pCouncil, m_spellId, true, 0, 0, me->GetGUID());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            DoZoneInCombat();
            uint32 damage = 0;
            SharedRule(damage);
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_blessingTimer < diff)
        {
            if(Unit *pUnit = SelectCouncil())
            {
                AddSpellToCast(pUnit, RAND(SPELL_BLESS_SPELLWARD, SPELL_BLESS_PROTECTION));
                m_blessingTimer = RAND(urand(15000, 20000), urand(25000, 35000));
            }
        }
        else
            m_blessingTimer -= diff;

        if (m_consecrationTimer < diff)
        {
            AddSpellToCast(m_creature, SPELL_CONSECRATION);
            m_consecrationTimer = urand(30000, 35000);
        }
        else
            m_consecrationTimer -= diff;

        if (m_hammerTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true, 0, 10.0f))
            {
                AddSpellToCast(pTarget, SPELL_HAMMER_OF_JUSTICE);
                m_hammerTimer = urand(10000, 20000);
            }
        }
        else
            m_hammerTimer -= diff;

        if (m_sealTimer < diff)
        {
            AddSpellToCast(m_creature, RAND(SPELL_SEAL_OF_COMMAND, SPELL_SEAL_OF_BLOOD));
            m_sealTimer = urand(17000, 20000);
        }
        else
            m_sealTimer -= diff;

        if (m_judgementTimer < diff)
        {
            ForceSpellCast(me->getVictim(), SPELL_GATHIOS_JUDGEMENT, INTERRUPT_AND_CAST);
            m_judgementTimer = 15000;
        }
        else
            m_judgementTimer -= diff;

        if (m_auraTimer < diff)
        {
            ApplyAura(RAND(SPELL_DEVOTION_AURA, SPELL_CHROMATIC_AURA));
            m_auraTimer = 60000;
        }
        else
            m_auraTimer -= diff;

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

// High Nethermancer Zerevor's AI
struct TRINITY_DLL_DECL boss_high_nethermancer_zerevorAI : public illidari_council_baseAI
{
    boss_high_nethermancer_zerevorAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_blizzardTimer;
    uint32 m_flamestrikeTimer;
    uint32 m_dampenTimer;
    uint32 m_aexpTimer;

    void Reset()
    {
        m_blizzardTimer = urand(12000, 20000);
        m_flamestrikeTimer = 3800;
        m_dampenTimer = 67200;
        m_aexpTimer = 3000;
        SetAutocast(SPELL_ARCANE_BOLT, 2000, true, AUTOCAST_TANK, 40.0f, true);

        m_checkTimer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            if (me->GetDistance2d(me->getVictim()) <= 40.0f)
                me->GetMotionMaster()->MoveIdle();
            else
                me->GetMotionMaster()->MoveChase(me->getVictim(), 40);

            uint32 damage = 0;
            SharedRule(damage);
            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_dampenTimer < diff)
        {
            ForceSpellCast(m_creature, SPELL_DAMPEN_MAGIC);
            m_dampenTimer = 67200;                      // almost 1,12 minutes (??)
        }
        else
            m_dampenTimer -= diff;

        if (m_blizzardTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                AddSpellToCast(pTarget, SPELL_BLIZZARD);
                m_blizzardTimer = urand(11000, 17000);
            }
        }
        else
            m_blizzardTimer -= diff;

        if (m_flamestrikeTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                AddSpellToCast(pTarget, SPELL_FLAMESTRIKE);
                m_flamestrikeTimer = urand(9000, 12000);
            }
        }
        else
            m_flamestrikeTimer -= diff;

        if (m_aexpTimer < diff)
        {
            std::list<HostilReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
            for (std::list<HostilReference*>::iterator i = m_threatlist.begin(); i!= m_threatlist.end();++i)
            {
                if (Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid()))
                {
                    if (pUnit->IsWithinDistInMap(me, 5) && pUnit->GetTypeId() == TYPEID_PLAYER && pUnit->isAlive() && !pUnit->IsImmunedToDamage(SPELL_SCHOOL_MASK_ARCANE))
                    {
                        ForceAOESpellCast(SPELL_ARCANE_EXPLOSION, INTERRUPT_AND_CAST);
                        m_aexpTimer = 3000;
                        break;
                    }
                    else
                        m_aexpTimer = 1000;
                }
            }
        }
        else
            m_aexpTimer -= diff;

        CastNextSpellIfAnyAndReady(diff);
    }
};

// Lady Malande's AI
struct TRINITY_DLL_DECL boss_lady_malandeAI : public illidari_council_baseAI
{
    boss_lady_malandeAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_smiteTimer;
    uint32 m_cohTimer;
    uint32 m_wrathTimer;
    uint32 m_shieldTimer;

    void Reset()
    {
        m_smiteTimer = 2000;
        m_cohTimer = 20000;
        m_wrathTimer = urand(8000,12000);
        m_shieldTimer = 15000;

        m_checkTimer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            uint32 damage = 0;
            SharedRule(damage);
            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_smiteTimer < diff)
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                AddSpellToCast(pTarget, SPELL_EMPOWERED_SMITE);
                m_smiteTimer = urand(5000, 9000);
            }
        }
        else
            m_smiteTimer -= diff;

        if (m_cohTimer < diff)
        {
            AddSpellToCast(m_creature, SPELL_CIRCLE_OF_HEALING);
            m_cohTimer = urand(19000, 23000);
        }
        else
            m_cohTimer -= diff;

        if (m_wrathTimer < diff)
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                AddSpellToCast(pTarget, SPELL_DIVINE_WRATH);
                m_wrathTimer = urand(15000, 30000);
            }
        }
        else
            m_wrathTimer -= diff;

        if (m_shieldTimer < diff)
        {
            AddSpellToCast(m_creature, SPELL_REFLECTIVE_SHIELD);
            m_shieldTimer = urand(40000, 55000);
        }
        else
            m_shieldTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

// Veras Darkshadow's AI
struct TRINITY_DLL_DECL boss_veras_darkshadowAI : public illidari_council_baseAI
{
    boss_veras_darkshadowAI(Creature *c) : illidari_council_baseAI(c){}

    uint32 m_vanishTimer;

    void Reset()
    {
        m_vanishTimer = urand(15000, 25000);
        m_checkTimer = 1000;
    }

    void OnAuraRemove(Aura* aur, bool stackRemove)
    {
        if(aur->GetId() == SPELL_VANISH)
        {
            DoResetThreat();
            DoStartMovement(me->getVictim());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_checkTimer < diff)
        {
            uint32 damage = 0;
            SharedRule(damage);
            DoZoneInCombat();
            m_checkTimer = 1000;
        }
        else
            m_checkTimer -= diff;

        if (m_vanishTimer < diff)
        {
            float x, y, z;
            ForceSpellCast(me, SPELL_VANISH, INTERRUPT_AND_CAST_INSTANTLY);
            ForceSpellCast(me, SPELL_DEADLY_POISON, INTERRUPT_AND_CAST_INSTANTLY);
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, 35, true))
            {
                pTarget->GetGroundPointAroundUnit(x, y, z, 5.0, 3.14*RAND(0, 1/6, 2/6, 3/6, 4/6, 5/6, 1));
                DoTeleportTo(x, y, z);
                m_creature->GetMotionMaster()->MoveIdle();
            }
            else
            {
                me->GetGroundPointAroundUnit(x, y, z, 30.0, 3.14*RAND(0, 1/6, 2/6, 3/6, 4/6, 5/6, 1));
                DoTeleportTo(x, y, z);
                m_creature->GetMotionMaster()->MoveIdle();
            }
            m_vanishTimer = 60000;
        }
        else
                m_vanishTimer -= diff;

        if(me->HasAura(SPELL_VANISH, 1))
            return;

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_mob_blood_elf_council_voice_trigger(Creature* c)
{
    return new mob_blood_elf_council_voice_triggerAI(c);
}

CreatureAI* GetAI_mob_illidari_council(Creature *_Creature)
{
    return new mob_illidari_councilAI (_Creature);
}

CreatureAI* GetAI_boss_gathios_the_shatterer(Creature *_Creature)
{
    return new boss_gathios_the_shattererAI (_Creature);
}

CreatureAI* GetAI_boss_lady_malande(Creature *_Creature)
{
    return new boss_lady_malandeAI (_Creature);
}

CreatureAI* GetAI_boss_veras_darkshadow(Creature *_Creature)
{
    return new boss_veras_darkshadowAI (_Creature);
}

CreatureAI* GetAI_boss_high_nethermancer_zerevor(Creature *_Creature)
{
    return new boss_high_nethermancer_zerevorAI (_Creature);
}

void AddSC_boss_illidari_council()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_illidari_council";
    newscript->GetAI = &GetAI_mob_illidari_council;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_blood_elf_council_voice_trigger";
    newscript->GetAI = &GetAI_mob_blood_elf_council_voice_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_gathios_the_shatterer";
    newscript->GetAI = &GetAI_boss_gathios_the_shatterer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_lady_malande";
    newscript->GetAI = &GetAI_boss_lady_malande;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_veras_darkshadow";
    newscript->GetAI = &GetAI_boss_veras_darkshadow;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_high_nethermancer_zerevor";
    newscript->GetAI = &GetAI_boss_high_nethermancer_zerevor;
    newscript->RegisterSelf();
}
