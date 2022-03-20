class infoContainer {
    constructor() {
        this.name = '';
        this.macAdr = '';
        this.temp = '';
        this.light = '';
    }

    get name(){
        return this.name? this.name : this.macAdr;
    }
}