
(function(){

this.MooTools = {
	version: '1.4.5',
	build: 'ab8ea8824dc3b24b6666867a2c4ed58ebb762cf0'
};

// typeOf, instanceOf

var typeOf = this.typeOf = function(item){
	if (item == null) return 'null';
	if (item.$family != null) return item.$family();

	if (item.nodeName){
		if (item.nodeType == 1) return 'element';
		if (item.nodeType == 3) return (/\S/).test(item.nodeValue) ? 'textnode' : 'whitespace';
	} else if (typeof item.length == 'number'){
		if (item.callee) return 'arguments';
		if ('item' in item) return 'collection';
	}

	return typeof item;
};

var instanceOf = this.instanceOf = function(item, object){
	if (item == null) return false;
	var constructor = item.$constructor || item.constructor;
	while (constructor){
		if (constructor === object) return true;
		constructor = constructor.parent;
	}
	/*<ltIE8>*/
	if (!item.hasOwnProperty) return false;
	/*</ltIE8>*/
	return item instanceof object;
};

// Function overloading

var Function = this.Function;

var enumerables = true;
for (var i in {toString: 1}) enumerables = null;
if (enumerables) enumerables = ['hasOwnProperty', 'valueOf', 'isPrototypeOf', 'propertyIsEnumerable', 'toLocaleString', 'toString', 'constructor'];

Function.prototype.overloadSetter = function(usePlural){
	var self = this;
	return function(a, b){
		if (a == null) return this;
		if (usePlural || typeof a != 'string'){
			for (var k in a) self.call(this, k, a[k]);
			if (enumerables) for (var i = enumerables.length; i--;){
				k = enumerables[i];
				if (a.hasOwnProperty(k)) self.call(this, k, a[k]);
			}
		} else {
			self.call(this, a, b);
		}
		return this;
	};
};

Function.prototype.overloadGetter = function(usePlural){
	var self = this;
	return function(a){
		var args, result;
		if (typeof a != 'string') args = a;
		else if (arguments.length > 1) args = arguments;
		else if (usePlural) args = [a];
		if (args){
			result = {};
			for (var i = 0; i < args.length; i++) result[args[i]] = self.call(this, args[i]);
		} else {
			result = self.call(this, a);
		}
		return result;
	};
};

Function.prototype.extend = function(key, value){
	this[key] = value;
}.overloadSetter();

Function.prototype.implement = function(key, value){
	this.prototype[key] = value;
}.overloadSetter();

// From

var slice = Array.prototype.slice;

Function.from = function(item){
	return (typeOf(item) == 'function') ? item : function(){
		return item;
	};
};

Array.from = function(item){
	if (item == null) return [];
	return (Type.isEnumerable(item) && typeof item != 'string') ? (typeOf(item) == 'array') ? item : slice.call(item) : [item];
};

Number.from = function(item){
	var number = parseFloat(item);
	return isFinite(number) ? number : null;
};

String.from = function(item){
	return item + '';
};

// hide, protect

Function.implement({

	hide: function(){
		this.$hidden = true;
		return this;
	},

	protect: function(){
		this.$protected = true;
		return this;
	}

});

// Type

var Type = this.Type = function(name, object){
	if (name){
		var lower = name.toLowerCase();
		var typeCheck = function(item){
			return (typeOf(item) == lower);
		};

		Type['is' + name] = typeCheck;
		if (object != null){
			object.prototype.$family = (function(){
				return lower;
			}).hide();
			//<1.2compat>
			object.type = typeCheck;
			//</1.2compat>
		}
	}

	if (object == null) return null;

	object.extend(this);
	object.$constructor = Type;
	object.prototype.$constructor = object;

	return object;
};

var toString = Object.prototype.toString;

Type.isEnumerable = function(item){
	return (item != null && typeof item.length == 'number' && toString.call(item) != '[object Function]' );
};

var hooks = {};

var hooksOf = function(object){
	var type = typeOf(object.prototype);
	return hooks[type] || (hooks[type] = []);
};

var implement = function(name, method){
	if (method && method.$hidden) return;

	var hooks = hooksOf(this);

	for (var i = 0; i < hooks.length; i++){
		var hook = hooks[i];
		if (typeOf(hook) == 'type') implement.call(hook, name, method);
		else hook.call(this, name, method);
	}

	var previous = this.prototype[name];
	if (previous == null || !previous.$protected) this.prototype[name] = method;

	if (this[name] == null && typeOf(method) == 'function') extend.call(this, name, function(item){
		return method.apply(item, slice.call(arguments, 1));
	});
};

var extend = function(name, method){
	if (method && method.$hidden) return;
	var previous = this[name];
	if (previous == null || !previous.$protected) this[name] = method;
};

Type.implement({

	implement: implement.overloadSetter(),

	extend: extend.overloadSetter(),

	alias: function(name, existing){
		implement.call(this, name, this.prototype[existing]);
	}.overloadSetter(),

	mirror: function(hook){
		hooksOf(this).push(hook);
		return this;
	}

});

new Type('Type', Type);

// Default Types

var force = function(name, object, methods){
	var isType = (object != Object),
		prototype = object.prototype;

	if (isType) object = new Type(name, object);

	for (var i = 0, l = methods.length; i < l; i++){
		var key = methods[i],
			generic = object[key],
			proto = prototype[key];

		if (generic) generic.protect();
		if (isType && proto) object.implement(key, proto.protect());
	}

	if (isType){
		var methodsEnumerable = prototype.propertyIsEnumerable(methods[0]);
		object.forEachMethod = function(fn){
			if (!methodsEnumerable) for (var i = 0, l = methods.length; i < l; i++){
				fn.call(prototype, prototype[methods[i]], methods[i]);
			}
			for (var key in prototype) fn.call(prototype, prototype[key], key)
		};
	}

	return force;
};

force('String', String, [
	'charAt', 'charCodeAt', 'concat', 'indexOf', 'lastIndexOf', 'match', 'quote', 'replace', 'search',
	'slice', 'split', 'substr', 'substring', 'trim', 'toLowerCase', 'toUpperCase'
])('Array', Array, [
	'pop', 'push', 'reverse', 'shift', 'sort', 'splice', 'unshift', 'concat', 'join', 'slice',
	'indexOf', 'lastIndexOf', 'filter', 'forEach', 'every', 'map', 'some', 'reduce', 'reduceRight'
])('Number', Number, [
	'toExponential', 'toFixed', 'toLocaleString', 'toPrecision'
])('Function', Function, [
	'apply', 'call', 'bind'
])('RegExp', RegExp, [
	'exec', 'test'
])('Object', Object, [
	'create', 'defineProperty', 'defineProperties', 'keys',
	'getPrototypeOf', 'getOwnPropertyDescriptor', 'getOwnPropertyNames',
	'preventExtensions', 'isExtensible', 'seal', 'isSealed', 'freeze', 'isFrozen'
])('Date', Date, ['now']);

Object.extend = extend.overloadSetter();

Date.extend('now', function(){
	return +(new Date);
});

new Type('Boolean', Boolean);

// fixes NaN returning as Number

Number.prototype.$family = function(){
	return isFinite(this) ? 'number' : 'null';
}.hide();

// Number.random

Number.extend('random', function(min, max){
	return Math.floor(Math.random() * (max - min + 1) + min);
});

// forEach, each

var hasOwnProperty = Object.prototype.hasOwnProperty;
Object.extend('forEach', function(object, fn, bind){
	for (var key in object){
		if (hasOwnProperty.call(object, key)) fn.call(bind, object[key], key, object);
	}
});

Object.each = Object.forEach;

Array.implement({

	forEach: function(fn, bind){
		for (var i = 0, l = this.length; i < l; i++){
			if (i in this) fn.call(bind, this[i], i, this);
		}
	},

	each: function(fn, bind){
		Array.forEach(this, fn, bind);
		return this;
	}

});

// Array & Object cloning, Object merging and appending

var cloneOf = function(item){
	switch (typeOf(item)){
		case 'array': return item.clone();
		case 'object': return Object.clone(item);
		default: return item;
	}
};

Array.implement('clone', function(){
	var i = this.length, clone = new Array(i);
	while (i--) clone[i] = cloneOf(this[i]);
	return clone;
});

var mergeOne = function(source, key, current){
	switch (typeOf(current)){
		case 'object':
			if (typeOf(source[key]) == 'object') Object.merge(source[key], current);
			else source[key] = Object.clone(current);
		break;
		case 'array': source[key] = current.clone(); break;
		default: source[key] = current;
	}
	return source;
};

Object.extend({

	merge: function(source, k, v){
		if (typeOf(k) == 'string') return mergeOne(source, k, v);
		for (var i = 1, l = arguments.length; i < l; i++){
			var object = arguments[i];
			for (var key in object) mergeOne(source, key, object[key]);
		}
		return source;
	},

	clone: function(object){
		var clone = {};
		for (var key in object) clone[key] = cloneOf(object[key]);
		return clone;
	},

	append: function(original){
		for (var i = 1, l = arguments.length; i < l; i++){
			var extended = arguments[i] || {};
			for (var key in extended) original[key] = extended[key];
		}
		return original;
	}

});

// Object-less types

['Object', 'WhiteSpace', 'TextNode', 'Collection', 'Arguments'].each(function(name){
	new Type(name);
});

// Unique ID

var UID = Date.now();

String.extend('uniqueID', function(){
	return (UID++).toString(36);
});

//<1.2compat>

var Hash = this.Hash = new Type('Hash', function(object){
	if (typeOf(object) == 'hash') object = Object.clone(object.getClean());
	for (var key in object) this[key] = object[key];
	return this;
});

Hash.implement({

	forEach: function(fn, bind){
		Object.forEach(this, fn, bind);
	},

	getClean: function(){
		var clean = {};
		for (var key in this){
			if (this.hasOwnProperty(key)) clean[key] = this[key];
		}
		return clean;
	},

	getLength: function(){
		var length = 0;
		for (var key in this){
			if (this.hasOwnProperty(key)) length++;
		}
		return length;
	}

});

Hash.alias('each', 'forEach');

Object.type = Type.isObject;

var Native = this.Native = function(properties){
	return new Type(properties.name, properties.initialize);
};

Native.type = Type.type;

Native.implement = function(objects, methods){
	for (var i = 0; i < objects.length; i++) objects[i].implement(methods);
	return Native;
};

var arrayType = Array.type;
Array.type = function(item){
	return instanceOf(item, Array) || arrayType(item);
};

this.$A = function(item){
	return Array.from(item).slice();
};

this.$arguments = function(i){
	return function(){
		return arguments[i];
	};
};

this.$chk = function(obj){
	return !!(obj || obj === 0);
};

this.$clear = function(timer){
	clearTimeout(timer);
	clearInterval(timer);
	return null;
};

this.$defined = function(obj){
	return (obj != null);
};

this.$each = function(iterable, fn, bind){
	var type = typeOf(iterable);
	((type == 'arguments' || type == 'collection' || type == 'array' || type == 'elements') ? Array : Object).each(iterable, fn, bind);
};

this.$empty = function(){};

this.$extend = function(original, extended){
	return Object.append(original, extended);
};

this.$H = function(object){
	return new Hash(object);
};

this.$merge = function(){
	var args = Array.slice(arguments);
	args.unshift({});
	return Object.merge.apply(null, args);
};

this.$lambda = Function.from;
this.$mixin = Object.merge;
this.$random = Number.random;
this.$splat = Array.from;
this.$time = Date.now;

this.$type = function(object){
	var type = typeOf(object);
	if (type == 'elements') return 'array';
	return (type == 'null') ? false : type;
};

this.$unlink = function(object){
	switch (typeOf(object)){
		case 'object': return Object.clone(object);
		case 'array': return Array.clone(object);
		case 'hash': return new Hash(object);
		default: return object;
	}
};

//</1.2compat>

})();


/*
---

name: Array

description: Contains Array Prototypes like each, contains, and erase.

license: MIT-style license.

requires: Type

provides: Array

...
*/

Array.implement({

	/*<!ES5>*/
	every: function(fn, bind){
		for (var i = 0, l = this.length >>> 0; i < l; i++){
			if ((i in this) && !fn.call(bind, this[i], i, this)) return false;
		}
		return true;
	},

	filter: function(fn, bind){
		var results = [];
		for (var value, i = 0, l = this.length >>> 0; i < l; i++) if (i in this){
			value = this[i];
			if (fn.call(bind, value, i, this)) results.push(value);
		}
		return results;
	},

	indexOf: function(item, from){
		var length = this.length >>> 0;
		for (var i = (from < 0) ? Math.max(0, length + from) : from || 0; i < length; i++){
			if (this[i] === item) return i;
		}
		return -1;
	},

	map: function(fn, bind){
		var length = this.length >>> 0, results = Array(length);
		for (var i = 0; i < length; i++){
			if (i in this) results[i] = fn.call(bind, this[i], i, this);
		}
		return results;
	},

	some: function(fn, bind){
		for (var i = 0, l = this.length >>> 0; i < l; i++){
			if ((i in this) && fn.call(bind, this[i], i, this)) return true;
		}
		return false;
	},
	/*</!ES5>*/

	clean: function(){
		return this.filter(function(item){
			return item != null;
		});
	},

	invoke: function(methodName){
		var args = Array.slice(arguments, 1);
		return this.map(function(item){
			return item[methodName].apply(item, args);
		});
	},

	associate: function(keys){
		var obj = {}, length = Math.min(this.length, keys.length);
		for (var i = 0; i < length; i++) obj[keys[i]] = this[i];
		return obj;
	},

	link: function(object){
		var result = {};
		for (var i = 0, l = this.length; i < l; i++){
			for (var key in object){
				if (object[key](this[i])){
					result[key] = this[i];
					delete object[key];
					break;
				}
			}
		}
		return result;
	},

	contains: function(item, from){
		return this.indexOf(item, from) != -1;
	},

	append: function(array){
		this.push.apply(this, array);
		return this;
	},

	getLast: function(){
		return (this.length) ? this[this.length - 1] : null;
	},

	getRandom: function(){
		return (this.length) ? this[Number.random(0, this.length - 1)] : null;
	},

	include: function(item){
		if (!this.contains(item)) this.push(item);
		return this;
	},

	combine: function(array){
		for (var i = 0, l = array.length; i < l; i++) this.include(array[i]);
		return this;
	},

	erase: function(item){
		for (var i = this.length; i--;){
			if (this[i] === item) this.splice(i, 1);
		}
		return this;
	},

	empty: function(){
		this.length = 0;
		return this;
	},

	flatten: function(){
		var array = [];
		for (var i = 0, l = this.length; i < l; i++){
			var type = typeOf(this[i]);
			if (type == 'null') continue;
			array = array.concat((type == 'array' || type == 'collection' || type == 'arguments' || instanceOf(this[i], Array)) ? Array.flatten(this[i]) : this[i]);
		}
		return array;
	},

	pick: function(){
		for (var i = 0, l = this.length; i < l; i++){
			if (this[i] != null) return this[i];
		}
		return null;
	},

	hexToRgb: function(array){
		if (this.length != 3) return null;
		var rgb = this.map(function(value){
			if (value.length == 1) value += value;
			return value.toInt(16);
		});
		return (array) ? rgb : 'rgb(' + rgb + ')';
	},

	rgbToHex: function(array){
		if (this.length < 3) return null;
		if (this.length == 4 && this[3] == 0 && !array) return 'transparent';
		var hex = [];
		for (var i = 0; i < 3; i++){
			var bit = (this[i] - 0).toString(16);
			hex.push((bit.length == 1) ? '0' + bit : bit);
		}
		return (array) ? hex : '#' + hex.join('');
	}

});

//<1.2compat>

Array.alias('extend', 'append');

var $pick = function(){
	return Array.from(arguments).pick();
};

//</1.2compat>


/*
---

name: String

description: Contains String Prototypes like camelCase, capitalize, test, and toInt.

license: MIT-style license.

requires: Type

provides: String

...
*/

String.implement({

	test: function(regex, params){
		return ((typeOf(regex) == 'regexp') ? regex : new RegExp('' + regex, params)).test(this);
	},

	contains: function(string, separator){
		return (separator) ? (separator + this + separator).indexOf(separator + string + separator) > -1 : String(this).indexOf(string) > -1;
	},

	trim: function(){
		return String(this).replace(/^\s+|\s+$/g, '');
	},

	clean: function(){
		return String(this).replace(/\s+/g, ' ').trim();
	},

	camelCase: function(){
		return String(this).replace(/-\D/g, function(match){
			return match.charAt(1).toUpperCase();
		});
	},

	hyphenate: function(){
		return String(this).replace(/[A-Z]/g, function(match){
			return ('-' + match.charAt(0).toLowerCase());
		});
	},

	capitalize: function(){
		return String(this).replace(/\b[a-z]/g, function(match){
			return match.toUpperCase();
		});
	},

	escapeRegExp: function(){
		return String(this).replace(/([-.*+?^${}()|[\]\/\\])/g, '\\$1');
	},

	toInt: function(base){
		return parseInt(this, base || 10);
	},

	toFloat: function(){
		return parseFloat(this);
	},

	hexToRgb: function(array){
		var hex = String(this).match(/^#?(\w{1,2})(\w{1,2})(\w{1,2})$/);
		return (hex) ? hex.slice(1).hexToRgb(array) : null;
	},

	rgbToHex: function(array){
		var rgb = String(this).match(/\d{1,3}/g);
		return (rgb) ? rgb.rgbToHex(array) : null;
	},

	substitute: function(object, regexp){
		return String(this).replace(regexp || (/\\?\{([^{}]+)\}/g), function(match, name){
			if (match.charAt(0) == '\\') return match.slice(1);
			return (object[name] != null) ? object[name] : '';
		});
	}

});


/*
---

name: Number

description: Contains Number Prototypes like limit, round, times, and ceil.

license: MIT-style license.

requires: Type

provides: Number

...
*/

Number.implement({

	limit: function(min, max){
		return Math.min(max, Math.max(min, this));
	},

	round: function(precision){
		precision = Math.pow(10, precision || 0).toFixed(precision < 0 ? -precision : 0);
		return Math.round(this * precision) / precision;
	},

	times: function(fn, bind){
		for (var i = 0; i < this; i++) fn.call(bind, i, this);
	},

	toFloat: function(){
		return parseFloat(this);
	},

	toInt: function(base){
		return parseInt(this, base || 10);
	}

});

Number.alias('each', 'times');

(function(math){
	var methods = {};
	math.each(function(name){
		if (!Number[name]) methods[name] = function(){
			return Math[name].apply(null, [this].concat(Array.from(arguments)));
		};
	});
	Number.implement(methods);
})(['abs', 'acos', 'asin', 'atan', 'atan2', 'ceil', 'cos', 'exp', 'floor', 'log', 'max', 'min', 'pow', 'sin', 'sqrt', 'tan']);


/*
---

name: Function

description: Contains Function Prototypes like create, bind, pass, and delay.

license: MIT-style license.

requires: Type

provides: Function

...
*/

Function.extend({

	attempt: function(){
		for (var i = 0, l = arguments.length; i < l; i++){
			try {
				return arguments[i]();
			} catch (e){}
		}
		return null;
	}

});

Function.implement({

	attempt: function(args, bind){
		try {
			return this.apply(bind, Array.from(args));
		} catch (e){}

		return null;
	},

	/*<!ES5-bind>*/
	bind: function(that){
		var self = this,
			args = arguments.length > 1 ? Array.slice(arguments, 1) : null,
			F = function(){};

		var bound = function(){
			var context = that, length = arguments.length;
			if (this instanceof bound){
				F.prototype = self.prototype;
				context = new F;
			}
			var result = (!args && !length)
				? self.call(context)
				: self.apply(context, args && length ? args.concat(Array.slice(arguments)) : args || arguments);
			return context == that ? result : context;
		};
		return bound;
	},
	/*</!ES5-bind>*/

	pass: function(args, bind){
		var self = this;
		if (args != null) args = Array.from(args);
		return function(){
			return self.apply(bind, args || arguments);
		};
	},

	delay: function(delay, bind, args){
		return setTimeout(this.pass((args == null ? [] : args), bind), delay);
	},

	periodical: function(periodical, bind, args){
		return setInterval(this.pass((args == null ? [] : args), bind), periodical);
	}

});

//<1.2compat>

delete Function.prototype.bind;

Function.implement({

	create: function(options){
		var self = this;
		options = options || {};
		return function(event){
			var args = options.arguments;
			args = (args != null) ? Array.from(args) : Array.slice(arguments, (options.event) ? 1 : 0);
			if (options.event) args = [event || window.event].extend(args);
			var returns = function(){
				return self.apply(options.bind || null, args);
			};
			if (options.delay) return setTimeout(returns, options.delay);
			if (options.periodical) return setInterval(returns, options.periodical);
			if (options.attempt) return Function.attempt(returns);
			return returns();
		};
	},

	bind: function(bind, args){
		var self = this;
		if (args != null) args = Array.from(args);
		return function(){
			return self.apply(bind, args || arguments);
		};
	},

	bindWithEvent: function(bind, args){
		var self = this;
		if (args != null) args = Array.from(args);
		return function(event){
			return self.apply(bind, (args == null) ? arguments : [event].concat(args));
		};
	},

	run: function(args, bind){
		return this.apply(bind, Array.from(args));
	}

});

if (Object.create == Function.prototype.create) Object.create = null;

var $try = Function.attempt;

//</1.2compat>


/*
---

name: Object

description: Object generic methods

license: MIT-style license.

requires: Type

provides: [Object, Hash]

...
*/

(function(){

var hasOwnProperty = Object.prototype.hasOwnProperty;

Object.extend({

	subset: function(object, keys){
		var results = {};
		for (var i = 0, l = keys.length; i < l; i++){
			var k = keys[i];
			if (k in object) results[k] = object[k];
		}
		return results;
	},

	map: function(object, fn, bind){
		var results = {};
		for (var key in object){
			if (hasOwnProperty.call(object, key)) results[key] = fn.call(bind, object[key], key, object);
		}
		return results;
	},

	filter: function(object, fn, bind){
		var results = {};
		for (var key in object){
			var value = object[key];
			if (hasOwnProperty.call(object, key) && fn.call(bind, value, key, object)) results[key] = value;
		}
		return results;
	},

	every: function(object, fn, bind){
		for (var key in object){
			if (hasOwnProperty.call(object, key) && !fn.call(bind, object[key], key)) return false;
		}
		return true;
	},

	some: function(object, fn, bind){
		for (var key in object){
			if (hasOwnProperty.call(object, key) && fn.call(bind, object[key], key)) return true;
		}
		return false;
	},

	keys: function(object){
		var keys = [];
		for (var key in object){
			if (hasOwnProperty.call(object, key)) keys.push(key);
		}
		return keys;
	},

	values: function(object){
		var values = [];
		for (var key in object){
			if (hasOwnProperty.call(object, key)) values.push(object[key]);
		}
		return values;
	},

	getLength: function(object){
		return Object.keys(object).length;
	},

	keyOf: function(object, value){
		for (var key in object){
			if (hasOwnProperty.call(object, key) && object[key] === value) return key;
		}
		return null;
	},

	contains: function(object, value){
		return Object.keyOf(object, value) != null;
	},

	toQueryString: function(object, base){
		var queryString = [];

		Object.each(object, function(value, key){
			if (base) key = base + '[' + key + ']';
			var result;
			switch (typeOf(value)){
				case 'object': result = Object.toQueryString(value, key); break;
				case 'array':
					var qs = {};
					value.each(function(val, i){
						qs[i] = val;
					});
					result = Object.toQueryString(qs, key);
				break;
				default: result = key + '=' + encodeURIComponent(value);
			}
			if (value != null) queryString.push(result);
		});

		return queryString.join('&');
	}

});

})();

//<1.2compat>

Hash.implement({

	has: Object.prototype.hasOwnProperty,

	keyOf: function(value){
		return Object.keyOf(this, value);
	},

	hasValue: function(value){
		return Object.contains(this, value);
	},

	extend: function(properties){
		Hash.each(properties || {}, function(value, key){
			Hash.set(this, key, value);
		}, this);
		return this;
	},

	combine: function(properties){
		Hash.each(properties || {}, function(value, key){
			Hash.include(this, key, value);
		}, this);
		return this;
	},

	erase: function(key){
		if (this.hasOwnProperty(key)) delete this[key];
		return this;
	},

	get: function(key){
		return (this.hasOwnProperty(key)) ? this[key] : null;
	},

	set: function(key, value){
		if (!this[key] || this.hasOwnProperty(key)) this[key] = value;
		return this;
	},

	empty: function(){
		Hash.each(this, function(value, key){
			delete this[key];
		}, this);
		return this;
	},

	include: function(key, value){
		if (this[key] == null) this[key] = value;
		return this;
	},

	map: function(fn, bind){
		return new Hash(Object.map(this, fn, bind));
	},

	filter: function(fn, bind){
		return new Hash(Object.filter(this, fn, bind));
	},

	every: function(fn, bind){
		return Object.every(this, fn, bind);
	},

	some: function(fn, bind){
		return Object.some(this, fn, bind);
	},

	getKeys: function(){
		return Object.keys(this);
	},

	getValues: function(){
		return Object.values(this);
	},

	toQueryString: function(base){
		return Object.toQueryString(this, base);
	}

});

Hash.extend = Object.append;

Hash.alias({indexOf: 'keyOf', contains: 'hasValue'});


/*
---

name: JSON

description: JSON encoder and decoder.

license: MIT-style license.

SeeAlso: <http://www.json.org/>

requires: [Array, String, Number, Function]

provides: JSON

...
*/

if (typeof JSON == 'undefined') this.JSON = {};

//<1.2compat>

JSON = new Hash({
	stringify: JSON.stringify,
	parse: JSON.parse
});

//</1.2compat>

(function(){

var special = {'\b': '\\b', '\t': '\\t', '\n': '\\n', '\f': '\\f', '\r': '\\r', '"' : '\\"', '\\': '\\\\'};

var escape = function(chr){
	return special[chr] || '\\u' + ('0000' + chr.charCodeAt(0).toString(16)).slice(-4);
};

JSON.validate = function(string){
	string = string.replace(/\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g, '@').
					replace(/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g, ']').
					replace(/(?:^|:|,)(?:\s*\[)+/g, '');

	return (/^[\],:{}\s]*$/).test(string);
};

JSON.jsn = function(obj){
	if (obj && obj.toJSON) obj = obj.toJSON();
	switch (typeOf(obj)){
		case 'string':
			return '"' + obj.replace(/[\x00-\x1f\\"]/g, escape) + '"\n';
		case 'array':
			return '[' + obj.map(JSON.jsn).clean() + ']\n';
		case 'object':
		case 'hash':
			var string = [];
			Object.each(obj, function(value, key){
				var json = JSON.jsn(value);
				if (json) string.push(JSON.jsn(key) + ':' + json);
			});
			return '{' + string + '}\n';
		case 'number': case 'boolean': return '' + obj;
		case 'null': return 'null';
	}

	return null;
};

JSON.encode = JSON.stringify ? function(obj){
	return JSON.stringify(obj);
} : function(obj){
	if (obj && obj.toJSON) obj = obj.toJSON();
	switch (typeOf(obj)){
		case 'string':
			return '"' + obj.replace(/[\x00-\x1f\\"]/g, escape) + '"';
		case 'array':
			return '[' + obj.map(JSON.encode).clean() + ']';
		case 'object': case 'hash':
			var string = [];
			Object.each(obj, function(value, key){
				var json = JSON.encode(value);
				if (json) string.push(JSON.encode(key) + ':' + json);
			});
			return '{' + string + '}';
		case 'number': case 'boolean': return '' + obj;
		case 'null': return 'null';
	}

	return null;
};
JSON.decode = function(string, secure){
	if (!string || typeOf(string) != 'string') return null;

	if (secure || JSON.secure){
		if (JSON.parse) return JSON.parse(string);
		if (!JSON.validate(string)) throw new Error('JSON could not decode the input; security is enabled and the value is not secure.');
	}

	return eval('(' + string + ')');
};

})();


//Храмцевич С.В.
function clean_html_txt(txt) {
	var t = new String(txt);
	t = t.replace(/&#8221;/g,'"');

	t = t.replace(/<[^>]*>/g," ");
	t = t.replace(/&[^;]{1,7};/g," ");
	t = t.replace(/\s{1,}/g," ");
//	t = t.replace(/^\s{1,}/g,"");
//	t = t.replace(/\s{1,}$/g," ");
	
	return t;
}

function clean_html_txt_hh_doom(txt) {
	var t = new String(txt);
	t = t.replace(/&#8221;/g,'"');

	t = t.replace(/<[^>]*>/g," ");
	t = t.replace(/&[^;]{1,7};/g," ");
	t = t.replace(/\s{1,}/g," ");
	t = t.replace(/^\s{1,}/g,"");
	t = t.replace(/\s{1,}$/g,"");
	
	return t;
}

function full_clean_html_txt(txt) {
	var t = new String(txt);
	t = t.replace(/&#8221;/g,'"');

	t = t.replace(/<[^>]*>/g," ");
	t = t.replace(/&[^;]{1,7};/g," ");
	t = t.replace(/\s{2,}/g," ");
	t = t.replace(/^\s{1,}/g,"");
	t = t.replace(/\s{1,}$/g,"");
	
	return t;
}

function HhDom(name,html) {
/*	
	function makeMap(str){
		var obj = {}, items = str.split(",");
		for ( var i = 0; i < items.length; i++ )
			obj[ items[i] ] = true;
		return obj;
	}
	
	// Special Elements (can contain anything)
	var special = makeMap("script,style");
*/

	this.childs = [];
	this.attrs = {};
	this.name = name ? name : "NoName";
	this.text = "";
	if( html ) {
		this.HTMLtoHhDom(html);
	}
}

HhDom.makeMap = function(str) 
{
	var obj = {}, items = str.split(",");
	for ( var i = 0; i < items.length; i++ )
		obj[ items[i] ] = true;
	return obj;
};

HhDom.prototype.toPlainText = function() 
{
	var o = "";
	
	function do_work(el) {
		if( HhDom.special[el.name] ) {
			return;
		}
		
		if(el.name == "TextNode") {
			o += el.text + "\n";
		}
		
		for (var i =0; i < el.childs.length; ++i) {
			do_work( el.childs[i]);
		}
	}
	
	do_work(this);
	
	return o;
};

HhDom.prototype.toPlainTextSpace = function() 
{
	var o = "";
	function do_work(el) {
		//bb.lg("el.name = " + el.name);
		if( HhDom.special[el.name] ) {
			return;
		}		
		
		if(el.name == "TextNode") {
			o += el.text + " ";
			//bb.lg(o);
		}
		
		for (var i =0; i < el.childs.length; ++i) {
			do_work( el.childs[i]);
		}
	}
	
	do_work(this);
	
	return o;
};


HhDom.special = HhDom.makeMap("script,style,noscript");

HhDom.prototype.appendChild = function(ch) {this.childs.push(ch);}
HhDom.prototype.createAddTextNode = function(txt,simple) {
	if( simple === undefined) simple = true
//bb.lg('createAddTextNode','['+txt+']')	
	var v = HhDom.createTextNode(txt,simple);
	if(v) {
		var t = v.text.substr(0,120)
		t = t.replace(/\s/g," ");
//bb.lg('createAddTextNode---','['+ t +']')	
		this.appendChild(v);
	}
		
	return v;
}

HhDom.createTextNode = function(txt,simple) {
	var v = false;
/*	
	txt = txt.replace(/<[^>]*>/g,"");
	txt = txt.replace(/&[^;]{1,7};/g,"");
	txt = txt.replace(/\s{2,}/g," ");
	txt = txt.replace(/^\s{1,}/g,"");
	txt = txt.replace(/\s{1,}$/g,"");
*/
	if( simple) txt = clean_html_txt_hh_doom(txt);
	
	if(txt && txt != ' ' && txt.length > 0) {
		v = new HhDom("TextNode");
		v.text = txt;
	}
	
	return v;
}

HhDom.prototype.getElementsByTagName = 	function(nm){
	var retnode = [];
	function do_work (el) {
		//bb.lg("Name of elements: " + el.name);
		if(el.name == nm) {
			retnode.push(el);
		}
		
		for (var i =0; i < el.childs.length; ++i) {
			do_work( el.childs[i]);
		}
	}
	do_work(this);
	return retnode;
}

HhDom.prototype.Find = 	function(ret_attr,tag,attr_name,attr_val){
	var mt = this.getElementsByTagName(tag);
	var rt = [];
	if( attr_val ) attr_val = attr_val.toLowerCase();
	for (var m = 0 ; m < mt.length; ++m) {
		if( attr_name ) {
			var vl = mt[m].attrs[attr_name];
			if( vl ) vl = vl.toLowerCase();
			
			if(vl == attr_val) {
				if(mt[m].attrs[ret_attr])
					rt.push(mt[m].attrs[ret_attr]);
			}
		} else {
			if(mt[m].attrs[ret_attr])
				rt.push(mt[m].attrs[ret_attr]);
		}
	}
	
	return rt;
}

HhDom.prototype.getElementsText = 	function(tag){
	var mt = this.getElementsByTagName(tag);
	var rt = '';
	for (var m = 0 ; m < mt.length; ++m) {
		rt += mt[m].toPlainText();
	}
	
	return rt;
}

HhDom.prototype.tree = function ()
{
	var o = '';
	var dom = this
    function lookAt(el, n)
    {
		var t = "";
		for (var i = 0; i < n; i++)
				t += "  ";
		var classs = ''
		var ks = ''
		var k = Object.keys(el.attrs)
		if( k.length) ks = ' = ' + k

		if( el.attrs)
		if( el.attrs.class) classs = ': ' + el.attrs.class
				
//                bb.lg(t + el.name + class + ks);
		o +=  t + el.name + classs + ks
		if( el.name == 'TextNode') {
			var tx = el.text.substr(0,120)
			tx = tx.replace(/\s/g," ");
			o += ' *** ' + tx
		}
		
		o += '\n'
		
		for (var i = 0; i < el.childs.length; i++)
		{
				lookAt(el.childs[i], n+1);
		}		
    }
    lookAt(dom, 0);
    
    return o
}

HhDom.prototype.HTMLtoHhDom = function(html){
	var startTag = 		/<([-A-Za-z0-9_]+)((?:\s+[-A-Za-z0-9_]+(?:\s*=\s*(?:(?:"[^"]*")|(?:'[^']*')|[^>\s]+))?)*)\s*(\/?)>/mi;
	var startTagG = 	/<([-A-Za-z0-9_!]+)((?:\s+[-A-Za-z0-9_:]+(?:\s*=\s*(?:(?:"[^"]*")|(?:'[^']*')|[^>\s]+))?)*)\s*(\/?)>/mig;
	var startTagNextG =	/<([-A-Za-z0-9_!]+)((?:\s+[-A-Za-z0-9_:]+(?:\s*=\s*(?:(?:"[^"]*")|(?:'[^']*')|[^>\s]+))?)*)\s*(\/?)>/mig;
	var scriptEndG = 	/<\/script>/mig;
	
//	var startTag = /<([-A-Za-z0-9_]+)[^>]*>/;
	var endTag =	/<\/([-A-Za-z0-9_]+)[^>]*>/;
	var endTagG =	/<\/([-A-Za-z0-9_]+)[^>]*>/g;
	
	var attr_reg = /([-A-Za-z0-9_:]+)(?:\s*=\s*(?:(?:"((?:\\.|[^"])*)")|(?:'((?:\\.|[^'])*)')|([^>\s]+)))?/gmi;
	var attr_get_reg = /([-A-Za-z0-9_:]+)(?:\s*)=(?:\s*)'?"?([^"']*)"?'?$/;
	
	var stack = [this];
	var p = html;
	var match;
	var match_end;
	var match_next;
	var p_n = p;
	var new_el;
	var i,j,k,ii;
	var last_stack;
	var ma,m;
	var ind_htm = 0;
	var ind_startTagG = 0
	
	stack.last = function(){
		return this[ this.length - 1 ];
	};
	
//bb.lg('sz',html.length)
//bb.lg('[<'+html+'>]')
	
	startTagG.lastIndex = 0;
	var last_pos = 0
//	while( (match = startTagG.exec(p)) ) {
	
	while( 1 ) {
		last_stack = stack.last();
		
		last_pos = startTagG.lastIndex
		endTagG.lastIndex = startTagG.lastIndex
	
		match = startTagG.exec(p)
		match_end = endTagG.exec(p)
		if( match && match_end) {
			if( match.index > match_end.index) { //конец тега (завершающий тег) впереди открывающего
				var tag_nm = match_end[1].toLowerCase();
				var txt_chank =  p.substring(last_pos,match_end.index);
				stack.last().createAddTextNode(txt_chank);
//				bb.lg('1')
				startTagG.lastIndex = endTagG.lastIndex
				
				for(i=(stack.length) -1 ; i >= 0 ; --i) {
					if( tag_nm == stack[i].name) {
						stack.length = i;
						break;
					}
				}
				
				continue
			}
		}
		
		if( !match) break //нет больше тегов - выходим
		
		if( match.index > last_pos){ // кусок от начала сканирования до открывающего тега
				stack.last().createAddTextNode(p.substring(last_pos,match.index));
//				bb.lg('2')
		}
	
		//--
		ind_startTagG = startTagG.lastIndex  //??
		new_el = new HhDom( match[1].toLowerCase() );
		last_stack.appendChild( new_el );
		stack.push(new_el);
		
		//-- attrs
		if(match[2]) {
			ma = match[2].match(attr_reg);
			for(ii=0; ii < ma.length; ++ii) {
				m = ma[ii].match(attr_get_reg);
				if(m)
					new_el.attrs[m[1].toLowerCase()] = m[2];
			}
		}
		
		//-- script
		if( new_el.name == 'script') {
			scriptEndG.lastIndex = ind_startTagG
			var scend = scriptEndG.exec(p)
			if( scend) {
				new_el.createAddTextNode(p.substring(ind_startTagG,scend.index),false);
//				bb.lg('3')

				startTagG.lastIndex = scriptEndG.lastIndex
			}
			stack.pop();
			continue
		}
		
		//-- text + next
		
		startTagNextG.lastIndex = ind_startTagG
		endTagG.lastIndex = ind_startTagG
		match_next = startTagNextG.exec(p)
		match_end = endTagG.exec(p)
		
		if( !match_end && !match_next) { //закончили - тегов нет больше никаких
			new_el.createAddTextNode(p.substring(ind_startTagG));
//			bb.lg('4')

			break;
		}
		
		if( !match_next && match_end) { // остались только завершающие теги
			do {
				var tag_nm = match_end[1].toLowerCase();
				var txt_chank =  p.substring(ind_startTagG,match_end.index);
				
				stack.last().createAddTextNode(txt_chank);
//				bb.lg('5')
				ind_startTagG = endTagG.lastIndex
				
				for(i=(stack.length) -1 ; i >= 0 ; --i) {
					if( tag_nm == stack[i].name) {
						stack.length = i;
						break;
					}
				}		
				
			} while (match_end = endTagG.exec(p))
			stack.last().createAddTextNode(p.substring(ind_startTagG));
//			bb.lg('6')
			
			break;
		}
		
		if( match_next && !match_end) { // нет больше завершающих тегов
			
			new_el.createAddTextNode(p.substring(ind_startTagG,match_next.index));
			startTagG.lastIndex = startTagNextG.lastIndex;
			//startTagG.lastIndex = match_next.index;
//			bb.lg('7')
			continue
		}
		
		if( match_next && match_end) {// обработка возможных закрывающих тегов
			
			do {
				if( !match_end) break // нет закрывающих больше
				if( match_next.index < match_end.index) break  // открывающий раньше закрывающего
				
				var tag_nm = match_end[1].toLowerCase();
				var txt_chank =  p.substring(startTagG.lastIndex,match_end.index);
				stack.last().createAddTextNode(txt_chank);
//				bb.lg('8')
				startTagG.lastIndex = endTagG.lastIndex
				
				for(i=(stack.length) -1 ; i >= 0 ; --i) {
					if( tag_nm == stack[i].name) {
						stack.length = i;
						break;
					}
				}		
				
			} while (match_end = endTagG.exec(p))
//			stack.last().createAddTextNode(p.substring(startTagG.lastIndex,match_next.index));// АНАЛИЗИРОВАТЬ !!!!
//			startTagG.lastIndex = endTagG.lastIndex
//			bb.lg('9')
			continue
		}
		
	}
}

//-- sorted array

Array.prototype.sortSearch = function(searchItem, compare, right)
{
    if (searchItem === undefined) return null;
    if (!compare) {
        compare = function(a, b)
        {
            return (String(a) == String(b)) ? 0 : (String(a) < String(b)) ? -1 : +1;
        }
    }
    var found = false, l = 0, u = this.length - 1;
    while (l <= u) {
        var m = parseInt((l + u) / 2);
        switch (compare(this[m], searchItem)) {
        case -1:
            var ml = m;
            l = m + 1;
            break;
        case +1:
            var mu = m;
            u = m - 1;
            break;
        default:
            found = true;
            if (right) {
                l = m + 1;
            } else {
                u = m - 1;
            }
        }
    }
    if (!found) {
        this.insertIndex = (ml + 1) || mu || 0;
        //this.insertIndex = (ml) ? ml + 1 : mu;
        return -1;
    }
    return (right) ? u : l;
}

Array.prototype.sortIndexOf = function(searchItem, compare)
{
    return this.sortSearch(searchItem, compare, false);
}

Array.prototype.sortLastIndexOf = function(searchItem, compare)
{
    return this.sortSearch(searchItem, compare, true);
}

Array.prototype.findSorted = function(w,exactly)
{
	var sa = this
//	var wr = RegExp('^' + w)
	
	if( exactly === undefined) exactly = false;
	var oa = new Array;
	
	if( w == '') return oa;
	
	var i = sa.sortIndexOf(w);
	if( i == -1 ) i = sa.insertIndex;
	
	for(; i < sa.length; ++i) {
		var ww = String(sa[i]);
		
		if( exactly) {
			if( ww == w) oa.push(ww);
			else break;
		} else {
			if( ww.indexOf(w) == 0 ) 
//			if( ww.match(wr) !== null ) 
				oa.push(ww);
			else break;
		}
	}
	
    return oa;
}

function find_sorted(w,sa,exactly)
{
	if( exactly === undefined) exactly = false;
	var oa = new Array;
	
	if( w == '') return oa;
	
	var i = sa.sortIndexOf(w);
	if( i == -1 ) i = sa.insertIndex;
	
	for(; i < sa.length; ++i) {
		var ww = String(sa[i]);
		
		if( exactly) {
			if( ww == w) oa.push(ww);
			else break;
		} else {
			if( ww.indexOf(w) == 0 ) oa.push(ww);
			else break;
		}
	}
	
    return oa;
}
//--

function clone(object)
{
    if (typeof(object) != "object") return object;
    var newObject = object.constructor();
    for (var objectItem in object) {
        newObject[objectItem] = clone(object[objectItem]);
    }
    return newObject;
}


function objectDisplay(obj) {
	 var o = "";
	 for (var prop in obj){
		o += obj.name + "." + prop + "=" + obj[prop] + "\n";
	}
	return o;
}

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

//function bloom(obj) {return false;}

function content_og_func() {
	js_print(link_html+ "\n\n");
	js_print(in_htmlPlainText+ "\n");

	content_og = dom.Find("content","meta","property","og:title");
    if( content_og.length > 0 ) {
        var ogtitle  = clean_html_txt(content_og[0]);
        var ogType = dom.Find("content","meta","property","og:type");
        if( ogType.length == 0 || ogType[0].toLowerCase() == 'article') return ogtitle;
        return '';
    }

	var title = dom.getElementsByTagName("title");
	if( title.length <= 0) return '';

	var tt = title[0].toPlainText();
	js_print(tt + "\n");
	if( tt.indexOf("301 Moved")  >= 0) {
		js_print(in_html + "\n");
	}



//	var l = tt.split(/[-—:\/\\=~#%;\x7c]/g);
	var l = tt.split(/(\s-\s)|—|---|--|:|\/|\\|=|~|#|%|;|\x7c/g);

	var o = [];
	for(var i=0; i < l.length; ++i ) {
		if( l[i] === undefined ) continue;
		var it = full_clean_html_txt(l[i]);
		if( it == '-') continue;

		js_print("(" + it + ")");


		var ind = in_htmlPlainText.indexOf(it);
		if( ind  == -1) continue;
		ind = in_htmlPlainText.indexOf(it,ind + it.length);
		if( ind  == -1) continue;

		if( !bloom(it)) {
			o.push(it);
		}

	}

	var r = full_clean_html_txt(o.join(' '));
	js_print(r + "\n");
	return (r);
//	return (o.join(' '));
//	return (o);
}

function noBloom_func() {
	var r = '';


	return r;
}

function isRedirect_func() {
	var r = '';

	var title = dom.getElementsByTagName("title");
	if( title.length <= 0) return r;

	var tt = title[0].toPlainText();
	if( tt.indexOf("301 Moved")  == 0) {
		aa = dom.Find("href","a");
		js_print("\nredirect : " + link_html + "\nto : " + aa[0] + "\n");
		return aa[0];
	}

	return r;
}


//dom = new HhDom("Test",in_html);
//in_htmlPlainText = dom.toPlainText();

function getRss_func() {
	var dom = new HhDom("Test",in_html);
	var aa = dom.getElementsByTagName("link");
	var tt = "";

	aa.each(function(a){
		tt += a.toPlainText();
	});

	return tt;
}

function getRssObj_func() {
	var o = [];
	var dom = new HhDom("Test",in_html);
	var aa = dom.getElementsByTagName("item");
	var tt = '';
	aa.each(function(a){
		tt += full_clean_html_txt(a.getElementsText('link')) + '^';
		tt += full_clean_html_txt(a.getElementsText('title')) + '^';
		tt += full_clean_html_txt(a.getElementsText('description')) + "\n";
		o.push({
			link: full_clean_html_txt(a.getElementsText('link')),
			title: full_clean_html_txt(a.getElementsText('title')),
			description: full_clean_html_txt(a.getElementsText('description'))
		});
	});

//	return o;
	return tt;
}

function sort_desc_fnc(a, b)
{
	return (String(a) == String(b)) ? 0 : (String(a) < String(b)) ? +1 : -1;
}


//JSON.encode(getRssObj_func());
//getRss_func();
//getRssObj_func();


function http_get_status(s)
{
    if ( typeof s !== "string") return false;
    var status_template = /\b(\d{3})\b.*/;
    if ( s.search(status_template) > -1) {
	var status = s.match(status_template)[1];
	return( Number(status))
    }
    return false;
}