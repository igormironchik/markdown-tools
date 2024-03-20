// TADS3 Example File
// Source: https://www.tads.org/t3doc/doc/t3QuickStart.htm

#charset "us-ascii"
#include <adv3.h>
#include <en_us.h>

versionInfo: GameID
    IFID = '558c20af-6559-477a-9f98-b7b4274cd304'
    name = 'The Best Burglar'
    byline = 'by Eric Eve'
    htmlByline = 'by <a href="mailto:eric.eve@hmc.ox.ac.uk">
                  Eric Eve</a>'
    version = '3'
    authorEmail = 'Eric Eve <eric.eve@hmc.ox.ac.uk>'
    desc = 'You are the world\'s best burglar faced with the greatest challenge
        of your felonious career.'
    htmlDesc = 'You are the world\'s best burglar faced with the greatest
        challenge of your felonious career.'
;

gameMain: GameMainDef
    /* the initial player character is 'me' */
    initialPlayerChar = me

    showIntro()
    {
        "<b>The Best Burglar</b>\nWell, you've got this far. Now it's just a
        quick nip inside the house and out again carrying the Orb of Ultimate
        Satisfaction, an object that no burglar has ever managed to steal
        before. If you can pull it off you're sure to win the Burglar of the
        Year Award, putting you at the pinnacle of your profession.\b";
    }

    showGoodbye()
    {
        "Thanks for playing! ";
    }
;

startRoom: OutdoorRoom 'Driveway'
    "The large red-brick Tudor house stands immediately to the north of this end
    of the driveway, while the drive back to the road where you left your
    getaway vehicle runs off though a belt of trees to the southwest."

    roomFirstDesc = "Here you are in the drive of Number 305 Erehwon Avenue,
        with the great house you've come to burgle standing just before you to
        the north. The drive back to the road where you left your getaway
        vehicle runs off though a belt of trees to the southwest."

    north = frontDoor
    in asExit(north)
    southwest = drive
;

+ me: Actor
    pcDesc = "You're Alexis Lightfinger, burglar extraordinaire, the most
        professional thief in the known universe; but you're on a job now, so
        you don't have time for the narcissistic indulgence of admiring your own
        appearance. You're far too professional not to have come fully prepared,
        so there's no practical need to look yourself over again. "
;

++ Container 'large white swag bag*bags' 'swag bag'
    "It's a large white bag with <q>SWAG</q> printed on it in very large
    letters. Everyone knows that no real burglar would ever carry such a thing,
    so by carrying it you know no one will take you for a real burglar. Cunning,
    eh? "
;

+ frontDoor: LockableWithKey, Door 'solid oak front door*doors' 'front door'
    "The lintel above the front door is carved with the date 1589, presumably
    the date the house was built. The door itself is made of solid oak. "
    keyList = [brassKey]

    makeOpen(stat)
    {
        inherited(stat);
        if(stat)
            achievement.awardPointsOnce();
    }

    achievement: Achievement { +10 "opening the front door" }

;

+ flowerPot: ComplexContainer 'terracotta small flower flowerpot/pot*pots'
    'flower pot'
    "It's a perfectly ordinary small terracota pot, though it looks like no
    one's got round to putting a plant in it yet. "
    subContainer: ComplexComponent, Container { bulkCapacity = 3}
    subUnderside: ComplexComponent, Underside { }

    initSpecialDesc = "A small flower pot rests on the ground not far from the
        front door. "

    bulk = 3
    bulkCapacity = 3
;

++ brassKey: Hidden, Key 'small brass key*keys' 'small brass key'
    "It's an ordinary enough small brass key. "
    subLocation = &subUnderside
;

+ Enterable -> frontDoor 'large red red-brick tudor house/mansion/front
    *houses*buildings' 'house'
    "It's a large red-brick Tudor house with mullioned windows, climbing
    creepers and the date 1589 carved over the door. "
;

++ Component '(door) carved lintel' 'lintel'
    "Its most noteworthy feature is the date 1589 carved into it. "
;

+ Decoration 'mullioned windows' 'windows'
    "They're architecturally attractive, no doubt, but not especially helpful to
    burglars. "

    notImportantMsg = 'It\'s a matter of professional pride with you never to
        mess with windows. '
    isPlural = true
;

+ Decoration 'green climbing ivy/creepers/creeper' 'creepers'
    "The front of the house is festooned with green creepers -- ivy, perhaps,
    but botany was never your strong point since in the main plants aren't
    worth burgling. "

    notImportantMsg = 'The creepers can\'t help you burgle the house -- they\'re
        certainly not strong enough to climb and they\'re certainly not worth
        stealing -- so you may as well leave them alone. '
    isPlural = true
;

+ drive: PathPassage 'drive/path/avenue' 'drive'
    "The drive leading back to the road runs off through a belt of trees to the
    southwest. "

    dobjFor(TravelVia)
    {
        action()
        {
            "You retrace your steps back to the road, where your trusty unmarked
            burglarmobile is still parked, ready for your quick getaway. ";

            if(orb.isIn(me))
            {
                "Congratulations! You have got away with the Orb of Ultimate
                Satisfaction, a feat never before performed. As you slip the orb
                onto the back seat of your car and climb into the driver's seat
                you tell yourself that you're now absolutely certain to win
                the Burglar of the Year Award!\b";

                achievement.awardPointsOnce();

                finishGameMsg(ftVictory, [finishOptionUndo,
                    finishOptionFullScore]);
            }
            else
            {
                "It's a shame you didn't manage to steal the orb, though.
                Without it you'll never win the Burglar of the Year Award
                now.\b";

                finishGameMsg(ftFailure, [finishOptionUndo]);
            }
        }
    }

    okayRubMsg = 'What -- all of it? That may take a while! '
    achievement: Achievement { +10 "getting away with the orb" }
;

+ Decoration 'belt/trees' 'trees'
    "The trees are in full leaf, which is good, because they hide what you're
    doing from the road. "

    notImportantMsg = 'The trees are doing a good job of hiding you from the
        road, so you may as well leave them alone. It\'s not as if they\'re
        something you could steal, after all. '
;

hallway: Room 'Hallway'
    "This hall is or grand proportions but pretty bare. The front door lies to
    the south and other exits lead east, north and west. "

    south = hallDoor
    out asExit(south)
    west = study
    north: FakeConnector { "You're pretty sure that only leads to the kitchen,
        and you haven't come here to cook a meal. " }

    east: DeadEndConnector { 'the living room' "You <<one of>>walk through the
        doorway and find yourself in<<or>>return to<<stopping>> the living room
        where you take <<one of>> a <<or>>another<<stopping>> quick look around,
        but <<one of>><<or>> once again<<stopping>> failing to find anything of
        interest you quickly return to the hall. "}
;

+ hallDoor: Lockable, Door -> frontDoor 'front door*doors' 'front door'
;

+ table:Surface 'small wooden mahogany side table/legs*tables' 'small table'
    "It's a small mahogany table standing on four thin legs. "
    initSpecialDesc = "A small table rests by the east wall. "
    bulk = 5

    dobjFor(Take)
    {
        check()
        {
            if(contents.length > 0)
                failCheck('It\'s probably not a very good idea to try picking
                    up the table while <<contents[1].nameIs>> still on it. ');
        }
    }
;

++ vase: Container 'cheap china floral vase/pattern' 'vase'
    "It's only a cheap thing, made of china but painted in a tasteless floral
    pattern using far too many primary colours. "

    bulk = 3
    bulkCapacity = 3
;

+++ silverKey: Hidden, Key 'small silver key*keys' 'small silver key'
;

study: Room 'Study'
    "This study is much as you would expect: somewhat spartan. A desk stands in
    the middle of the room with a chair placed just behind it. A <<if
      picture.moved>>safe is built into <<else>> rather bland painting hangs on
    <<end>> the west wall. The way out is to the east. "
    east = hallway
    out asExit(east)
;

+ desk: Heavy, Platform 'plain wooden desk' 'desk'
    "It's a plain wooden desk with a single drawer. "
    dobjFor(Open) remapTo(Open, drawer)
    dobjFor(Close) remapTo(Close, drawer)
    dobjFor(LookIn) remapTo(LookIn, drawer)
    dobjFor(UnlockWith) remapTo(UnlockWith, drawer, IndirectObject)
    dobjFor(LockWith) remapTo(LockWith, drawer, IndirectObject)
    dobjFor(Lock) remapTo(Lock, drawer)
    dobjFor(Unlock) remapTo(Unlock, drawer)
;

++ drawer: KeyedContainer, Component '(desk) drawer*drawers' 'drawer'
    "It's an ordinary desk drawer with a small silver lock. "
    keyList = [silverKey]
;

+++ notebook: Readable 'small bright red notebook/book/cover/pages'
    'small red notebook'
    "It's a small notebook with a bright red cover. "

    readDesc = "You open the notebook and flick through its pages. The only
        thing you find of any interest is a page with <q>SAFE DATE</q> scrawled
        across it. After satisfying yourself that the notebook contains nothing
        else of any potential relevance you snap it shut again. <.reveal
        safe-date>"

    dobjFor(Open) asDobjFor(Read)
    dobjFor(LookIn) asDobjFor(Read)

    dobjFor(Read)
    {
        action()
        {
            inherited;
            achievement.awardPointsOnce();
        }
    }

    cannotCloseMsg = 'It\'s already closed. '
    achievement: Achievement { +5 "reading the notebook" }
;

+ CustomImmovable, Chair 'red office swivel chair' 'chair'
    "It's a typical office swivel chair, covered in red fabric. "

    cannotTakeMsg = 'You see no reason to burden yourself with such a useless
        object; that would be quite unprofessional. '

;

+ picture: RoomPartItem, Thing 'rather bland picture/painting/landscape'
    'picture'
    "It's a landscape, pleasantly executed enough, but of no great distinction
    and definitely not worth the bother of stealing. "

    initNominalRoomPartLocation = defaultWestWall
    initSpecialDesc = "A rather bland painting hangs on the west wall. "
    isListed = (moved)

    bulk = 8

    dobjFor(LookBehind)
    {
        action()
        {
            if(moved)
                inherited;
            else
            {
                safe.discover();
                "Behind the picture is a safe built into the wall. ";
            }
        }
    }

    moveInto(newDest)
    {
        if(!safe.discovered)
        {
            "Removing the painting from the wall reveals a safe behind. ";
            safe.discover();
        }
        inherited(newDest);
    }
;

+ safe: RoomPartItem, Hidden, CustomFixture, ComplexContainer
    'sturdy steel safe' 'safe'
    "It's a sturdy steel safe with a single dial on its door. "

    subContainer: ComplexComponent, IndirectLockable, OpenableContainer
    {
        bulkCapacity = 5
        makeOpen(stat)
        {
            inherited(stat);
            if(stat)
                achievement.awardPointsOnce();
        }

        achievement: Achievement { +10 "opening the safe" }
    }

    specialDesc = "A safe is built into the west wall. "
    specialNominalRoomPartLocation = defaultWestWall
    cannotTakeMsg = 'It's firmly built into the wall; you can't budge it. '

    discover()
    {
        if(!discovered)
        {
            foreach(local cur in allContents)
                cur.discover();

            achievement.awardPointsOnce();
        }
        inherited();
    }

    achievement: Achievement { +5 "finding the safe" }
;

++ safeDoor:  Hidden, ContainerDoor '(safe) door' 'safe door'
    "It has a circular dial attached to its centre. "
;

+++ safeDial: Hidden, Component,  NumberedDial 'circular dial*dials' 'dial'
    "The dial can be turned to any number between <<minSetting>> and
    <<maxSetting>>. It's currently at <<curSetting>>. "

    minSetting = 0
    maxSetting = 99
    curSetting = '35'

    num1 = 0
    num2 = 0
    correctCombination = 1589

    makeSetting(val)
    {
        inherited(val);
        num2 = num1;
        num1 = toInteger(val);
        if(100 * num2 + num1 == correctCombination)
        {
            "You hear a slight <i>click</i> come from the safe door. ";
            safe.makeLocked(nil);
        }
        else if(!safe.isOpen)
            safe.makeLocked(true);
    }
;

++ orb: Thing 'ultimate battered dull metal orb/sphere/ball/satisfaction'
    'Orb of Ultimate Satisfaction'
    "It doesn't look much be honest, just a battered sphere made of some dull
    metal, but you've been told it's the most valuable and desirable object
    in the known universe! "

    aName = (theName)

    subLocation = &subContainer

    okayRubMsg = 'As {you/he} rub{s} {the dobj/him} a shimmering djiin suddenly
        appears in the air before you!\b
        <q>Hello, you have reached the automated holographic answering service
        of Jeannie the Genie,</q> she announces. <q>I\'m sorry I\'m not
        available to respond to your rub in person right now, but my hours of
        activity have been heavily curtailed by the European Working Time
        Directive. Before making a wish, please make sure that you have
        conducted a full risk assessment in line with the latest Health and
        Safety Guidelines. Also, please note that before any wish can be granted
        you must sign a Form P45/PDQ/LOL indemnifying this wish-granting agency
        against any consequential loss or damage arising from the fulfilment of
        your desires. Thank you for rubbing. Have a nice day!</q>\b
        Her message complete, the holographic djiin fades away into
        non-existence. '

    moveInto(dest)
    {
        inherited(dest);
        if(dest.isOrIsIn(me))
            achievement.awardPointsOnce();
    }

    achievement: Achievement { +10 "taking the orb" }
;

//------------------------------------------------------------------------------

/* DEFINE A NEW VERB */

DefineTAction(Rub)
;

VerbRule(Rub)
    'rub' dobjList
    : RubAction
    verbPhrase = 'rub/rubbing (what)'
;

/* When creating a new verb, you'll want to modify the Thing class so as to provide
   default handling for the command. The defaults specified here will be used except
   on objects for which you define explicit handling of the command. */

modify Thing
    dobjFor(Rub)
    {
        preCond = [touchObj]
        action() { mainReport(okayRubMsg); }
    }

    okayRubMsg = '{You/he} rub{s} {the dobj/him} but not much happens as a
        result. '

    shouldNotBreakMsg = 'Only amateurs go round breaking things unnecessarily. '
;

//------------------------------------------------------------------------------

/* HINTS */

TopHintMenu;

+ Goal -> (frontDoor.achievement)
    'How do I get into the house?'
    [
        'Well, the windows don\'t seem a good way in. ',
        'So perhaps you\'d better try the front door. ',
        'Could someone have left a key around somewhere? ',
        'Is there anything lying around where someone could have hidden a key? ',
        'What about that flowerpot? ',
        'Try looking under the flowerpot. '
    ]

    goalState = OpenGoal
;

/* The closeWhenSeen property of the following Goal object is an example of how to
   make your hint menu respond dynamically to the player's current situation. */

+ Goal 'Where can I find the orb? '
    [
        'Something like that is bound to be kept safe. ',
        'So it\'s probably inside the house. '
    ]

    goalState = OpenGoal
    closeWhenSeen = hallway
;

+ Goal 'Where can I find the orb?'
    [
        'It\'s sure to be kept somewhere safe. ',
        'You\'d better hunt around. ',
        'Somewhere in the study seems the most likely place. ',
        deskHint,
        'But it should be safely locked in a safe ',
        'Where might someone hide a safe in this study? ',
        'What could be behind that picture on the wall? ',
        'Try looking behind the picture (or simply taking the picture). '
    ]

    openWhenSeen = hallway
    closeWhenSeen = orb
;

++ deskHint: Hint 'Have you tried looking in the desk drawer? '
    [deskGoal]
;

+ deskGoal: Goal 'How do I get the desk drawer open?'
    [
        'Have you examined the drawer? ',
        'What might you need to unlock it? ',
        'Where might you find such a thing? ',
        'What have you seen that a small key might be hidden in? ',
        'How carefully have you searched the hall? ',
        'What is (or was) on the hall table? ',
        'What might that vase be for? ',
        'Try looking in the vase. '
    ]
    closeWhenSeen = notebook
;

+ Goal 'How do I get the safe open?'
    [
        'How carefully have you examined the safe? ',
        'Where might someone leave a clue to the combination? ',
        deskHint,
        'Make sure you read the notebook. ',
        'Once you\'ve found the combination you need to use the dial. ',
        'If the combination is a number larger than 99 you\'ll need to enter it
        in stages. ',
        'For example, if the combination were 1234 you\'d first need to turn the
        dial to 12 and then turn it to 34. '
    ]

    openWhenSeen = safe
    closeWhenAchieved = (safe.subContainer.achievement)
;

+ Goal 'What does the clue in the notebook mean?'
    [
        'Well, <q>SAFE</q> might refer to something you want to open. ',
        'Have you seen a date round here? ',
        'When was this house built? ',
        'Where might you find the year in which this house was built? ',
        'How carefully have you looked at the front of the house? ',
        'Did you examine the door? '
    ]

    openWhenRevealed = 'safe-date'
    closeWhenAchieved = (safe.subContainer.achievement)
;


+ Goal 'What do I do with the orb now I\'ve got it?'
    [
        'Well, you could try rubbing it. ',
        'But the main thing to do now is to escape with it. '
    ]
    openWhenSeen = orb
;
