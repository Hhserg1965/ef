function unixTime(d)
{
    var foo = new Date(d); // Generic JS date object
    return (parseInt(foo.getTime() / 1000));
}

function curUnixTime()
{
    var foo = new Date(); // Generic JS date object
    return (parseInt(foo.getTime() / 1000));
}
