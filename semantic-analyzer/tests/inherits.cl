class A inherits B {

};

class B inherits A { -- inheritance loop

};

class C inherits D { -- unknown class D

};

class P {
    f(): Int { 1 };
};
class W inherits P {
    f(): Int { "1" };
};


-- no Main class