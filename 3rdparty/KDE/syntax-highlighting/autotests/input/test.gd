tool
extends "res://Level.gd"
class_name MyNode

breakpoint
var x = preload()
const y = 1

onready var z = get_node("/root/Node")
export var r = 1

var value  = 1 setget setterfunc, getterfunc

class Test:
    func test(parameter):
        var c = CubeMesh.new()

enum {A, B, C}


static func _init():
    assert(PI == TAU)

    if INF == INF:
        pass
    elif 1. <= 1:
        pass
    else
        pass

signal moved():
    pass

remote func test_connection(_info):
    pass

master func test_connection(_info):
    pass

puppet func test_connection(_info):
    pass

remotesync func test_connection(_info):
    pass

mastersync func test_connection(_info):
    return 1

puppetsync func test_connection(_info):
    yield 2

func test():
    if 1 is 2 and 1 not is 1 or 1 in [1, "test", @"Node", $NodeName]:
        var x =  $NodeName as Node2D

    for i in range(1, 100):
        continue

    x.lala().prolog()
    x.lala().prolog
    x.lala
    x.lala()

    while 1 != 1:
        statement(s)
        break

    match typeof(123):
        1234:
            print("""test""")

    min(abc);
    Color8(abc);
    ColorN(abc);
    abs(abc);
    acos(abc);
    asin(abc);
    atan(abc);
    atan2(abc);
    bytes2var(abc);
    cartesian2polar(abc);
    ceil(abc);
    char(abc);
    clamp(abc);
    convert(abc);
    cos(abc);
    cosh(abc);
    db2linear(abc);
    decimals(abc);
    dectime(abc);
    deg2rad(abc);
    dict2inst(abc);
    ease(abc);
    exp(abc);
    floor(abc);
    fmod(abc);
    fposmod(abc);
    funcref(abc);
    get_stack(abc);
    hash(abc);
    inst2dict(abc);
    instance_from_id(abc);
    inverse_lerp(abc);
    is_equal_approx(abc);
    is_inf(abc);
    is_instance_valid(abc);
    is_nan(abc);
    is_zero_approx(abc);
    len(abc);
    lerp(abc);
    lerp_angle(abc);
    linear2db(abc);
    load(abc);
    log(abc);
    max(abc);
    min(abc);
    move_toward(abc);
    nearest_po2(abc);
    ord(abc);
    parse_json(abc);
    polar2cartesian(abc);
    posmod(abc);
    pow(abc);
    print(abc);
    print_debug(abc);
    print_stack(abc);
    printerr(abc);
    printraw(abc);
    prints(abc);
    printt(abc);
    push_error(abc);
    push_warning(abc);
    rad2deg(abc);
    rand_range(abc);
    rand_seed(abc);
    randf(abc);
    randi(abc);
    randomize(abc);
    range(abc);
    range_lerp(abc);
    round(abc);
    seed(abc);
    sign(abc);
    sin(abc);
    sinh(abc);
    smoothstep(abc);
    sqrt(abc);
    step_decimals(abc);
    stepify(abc);
    str(abc);
    str2var(abc);
    tan(abc);
    tanh(abc);
    to_json(abc);
    type_exists(abc);
    typeof(abc);
    validate_json(abc);
    var2bytes(abc);
    var2str(abc);
    weakref(abc);
    wrapf(abc);
    wrapi(abc);


    var x = null
    x = true
    x = false
    self.x = x
    self.x.connect("asd", x, "asd")

func _get(abc):
    pass

func _get_property_list(abc):
    pass

func _init(abc):
    pass

func _notification(abc):
    pass

func _set(abc):
    pass

func _to_string(abc):
    pass

func _enter_tree(abc):
    pass

func _exit_tree(abc):
    pass

func _get_configuration_warning(abc):
    pass

func _input(abc):
    pass

func _physics_process(abc):
    pass

func _process(abc):
    pass

func _ready(abc):
    pass

func _unhandled_input(abc):
    pass

func _unhandled_key_input(abc):
    pass





















