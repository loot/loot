(function(){
window.M8 = {data:{}};
(function(){
/**
 * modul8 v0.13.0
 */

var config    = {"namespace":"M8","domains":["app","shims","all","utils"],"arbiters":{},"logging":1}
  , ns        = window[config.namespace]
  , domains   = config.domains
  , arbiters  = []
  , exports   = {}
  , DomReg    = /^([\w]*)::/;

/**
 * Initialize the exports container with domain names + move data to it
 */
exports.M8 = {};
exports.external = {};
exports.data = ns.data;
delete ns.data;

domains.forEach(function(e){
  exports[e] = {};
});

/**
 * Attach arbiters to the require system then delete them from the global scope
 */
Object.keys(config.arbiters).forEach(function(name){
  var arbAry = config.arbiters[name];
  arbiters.push(name);
  exports.M8[name] = window[arbAry[0]];
  arbAry.forEach(function(e){
    delete window[e];
  });
});

/**
 * Converts a relative path to an absolute one
 */
function toAbsPath(pathName, relReqStr) {
  var folders = pathName.split('/').slice(0, -1);
  while (relReqStr.slice(0, 3) === '../') {
    folders = folders.slice(0, -1);
    relReqStr = relReqStr.slice(3);
  }
  return folders.concat(relReqStr.split('/')).join('/');
}

/**
 * Require Factory for ns.define
 * Each (domain,path) gets a specialized require function from this
 */
function makeRequire(dom, pathName) {
  return function(reqStr) {
    var o, scannable, k, skipFolder;

    if (config.logging >= 4) {
      console.debug('modul8: '+dom+':'+pathName+" <- "+reqStr);
    }

    if (reqStr.slice(0, 2) === './') {
      scannable = [dom];
      reqStr = toAbsPath(pathName, reqStr.slice(2));
    }
    else if (reqStr.slice(0,3) === '../') {
      scannable = [dom];
      reqStr = toAbsPath(pathName, reqStr);
    }
    else if (DomReg.test(reqStr)) {
      scannable = [reqStr.match(DomReg)[1]];
      reqStr = reqStr.split('::')[1];
    }
    else if (arbiters.indexOf(reqStr) >= 0) {
      scannable = ['M8'];
    }
    else {
      scannable = [dom].concat(domains.filter(function(e) {return e !== dom;}));
    }

    reqStr = reqStr.split('.')[0];
    if (reqStr.slice(-1) === '/' || reqStr === '') {
      reqStr += 'index';
      skipFolder = true;
    }

    if (config.logging >= 3) {
      console.log('modul8: '+dom+':'+pathName+' <- '+reqStr);
    }
    if (config.logging >= 4) {
      console.debug('modul8: scanned '+JSON.stringify(scannable));
    }

    for (k = 0; k < scannable.length; k += 1) {
      o = scannable[k];
      if (exports[o][reqStr]) {
        return exports[o][reqStr];
      }
      if (!skipFolder && exports[o][reqStr + '/index']) {
        return exports[o][reqStr + '/index'];
      }
    }

    if (config.logging >= 1) {
      console.error("modul8: Unable to resolve require for: " + reqStr);
    }
  };
}

ns.define = function(name, domain, fn) {
  var module = {};
  fn(makeRequire(domain, name), module, exports[domain][name] = {});
  if (module.exports) {
    delete exports[domain][name];
    exports[domain][name] = module.exports;
  }
};

/**
 * Public Debug API
 */

ns.inspect = function(domain) {
  console.log(exports[domain]);
};

ns.domains = function() {
  return domains.concat(['external','data']);
};

ns.require = makeRequire('app', 'CONSOLE');

/**
 * Live Extension API
 */

ns.data = function(name, exported) {
  if (exports.data[name]) {
    delete exports.data[name];
  }
  if (exported) {
    exports.data[name] = exported;
  }
};

ns.external = function(name, exported) {
  if (exports.external[name]) {
    delete exports.external[name];
  }
  if (exported) {
    exports.external[name] = exported;
  }
};

})();

// shared code

M8.define('index','utils',function(require, module, exports){
var hasOwnProperty = Object.prototype.hasOwnProperty;


var HTMLNames = [
    "HTMLDocument", "HTMLLinkElement", "HTMLElement", "HTMLHtmlElement", 
    "HTMLDivElement", "HTMLAnchorElement", "HTMLSelectElement", 
    "HTMLOptionElement", "HTMLInputElement", "HTMLHeadElement", 
    "HTMLSpanElement", "XULElement", "HTMLBodyElement", "HTMLTableElement", 
    "HTMLTableCellElement", "HTMLTextAreaElement", "HTMLScriptElement", 
    "HTMLAudioElement", "HTMLMediaElement", "HTMLParagraphElement", 
    "HTMLButtonElement", "HTMLLIElement", "HTMLUListElement", 
    "HTMLFormElement", "HTMLHeadingElement", "HTMLImageElement", 
    "HTMLStyleElement", "HTMLTableRowElement", "HTMLTableSectionElement", 
    "HTMLBRElement"
];

module.exports = {
	addShimToInterface: addShimToInterface,
	throwDOMException: throwDOMException,
	clone: clone,
    recursivelyWalk: recursivelyWalk,
	HTMLNames: HTMLNames
};

function recursivelyWalk(nodes, cb) {
    for (var i = 0, len = nodes.length; i < len; i++) {
        var node = nodes[i];
        var ret = cb(node);
        if (ret) {
            return ret;
        }
        if (node.childNodes && node.childNodes.length) {
            var ret = recursivelyWalk(node.childNodes, cb);
            if (ret) {
                return ret;
            }
        }
    }
}

function throwDOMException(code) {
    var ex = Object.create(DOMException.prototype);
    ex.code = code;
    throw ex;
}

function addShimToInterface(shim, proto, constructor) {
	Object.keys(shim).forEach(function _eachShimProperty(name) {
		if (name === "constants") {
			var constants = shim[name];
			Object.keys(constants).forEach(function _eachConstant(name) {
				if (!hasOwnProperty.call(constructor, name)) {
					constructor[name] = constants[name];	
				}
			});
			return;
		}

		if (!hasOwnProperty.call(proto, name)) {
			var pd = shim[name];
			if (pd.value) {
				pd.writable = false;
			} else {
                
            }
			pd.configurable = true;
            pd.enumerable = false; 
			Object.defineProperty(proto, name, pd);	
		}
	});
}

function clone(node, document, deep) {
    document = document || node.ownerDocument;
    var copy;
    if (node.nodeType === Node.ELEMENT_NODE) {
        var namespace = node.nodeName;
        if (node.prefix) {
            namespace = node.prefix + ":" + namespace;
        }
        copy = document.createElementNS(node.namespaceURI, namespace);
        for (var i = 0, len = node.attributes.length; i < len; i++) {
            var attr = node.attributes[i];
            copy.setAttribute(attr.name, attr.value);
        }
    } else if (node.nodeType === Node.DOCUMENT_NODE) {
        copy = document.implementation.createDocument("", "", null);
    } else if (node.nodeType === Node.DOCUMENT_FRAGMENT_NODE) {
        copy = document.createDocumentFragment();
    } else if (node.nodeType === Node.DOCUMENT_TYPE_NODE) {
        copy = document.implementation.createDocumentType(node.name, node.publicId, node.systemId);
    } else if (node.nodeType === Node.COMMENT_NODE) {
        copy = document.createComment(node.data);
    } else if (node.nodeType === Node.TEXT_NODE) {
        copy = document.createTextNode(node.data);
    } else if (node.nodeType === Node.PROCESSING_INSTRUCTION_NODE) {
        copy = document.createProcessingInstruction(node.target, node.data);
    }
    // TODO: other cloning steps from other specifications
    if (deep) {
        var children = node.childNodes;
        for (var i = 0, len = children.length; i < len; i++) {
            copy.appendChild(children[i].cloneNode(node, document, deep));
        }
    }
    return copy;
}
});
M8.define('interfaces/DOMTokenList','all',function(require, module, exports){
var utils = require("utils::index");

var throwDOMException = utils.throwDOMException;

module.exports = {
    constructor: DOMTokenList,
    item: item,
    contains: contains,
    add: add,
    remove: remove,
    toggle: toggle,
    toString: _toString
};

module.exports.constructor.prototype = module.exports;

function DOMTokenList(getter, setter) {
    this._getString = getter;
    this._setString = setter;
    fixIndex(this, getter().split(" "));
}

function fixIndex(clist, list) {
    for (var i = 0, len = list.length; i < len; i++) {
        clist[i] = list[i];
    }
    delete clist[len];
}

function handleErrors(token) {
    if (token === "" || token === undefined) {
        throwDOMException(DOMException.SYNTAX_ERR);
    }
    // TODO: test real space chacters
    if (token.indexOf(" ") > -1) {
        throwDOMException(DOMException.INVALID_CHARACTER_ERR);
    }
}

function getList(clist) {
    var str = clist._getString();
    if (str === "") {
        return [];
    } else {
        return str.split(" ");
    }
}

function item(index) {
    if (index >= this.length) {
        return null;
    }
    return this._getString().split(" ")[index];
}

function contains(token) {
    handleErrors(token);
    var list = getList(this);
    return list.indexOf(token) > -1;
}

function add(token) {
    handleErrors(token);
    var list = getList(this);
    if (list.indexOf(token) > -1) {
        return;
    }
    list.push(token);
    this._setString(list.join(" ").trim());
    fixIndex(this, list);
}

function remove(token) {
    handleErrors(token);
    var list = getList(this);
    var index = list.indexOf(token);
    if (index > -1) {
        list.splice(index, 1);
        this._setString(list.join(" ").trim());
    }
    fixIndex(this, list);
}

function toggle(token) {
    if (this.contains(token)) {
        this.remove(token);
        return false;
    } else {
        this.add(token);
        return true;
    }
}

function _toString() {
    return this._getString();
}
});
M8.define('interfaces/Element','all',function(require, module, exports){
var DOMTokenList = require("all::interfaces/DOMTokenList").constructor;

module.exports = {
	parentElement: {
		get: getParentElement
	},
    classList: {
        get: getClassList
    }
}

function getParentElement() {
    var parent = this.parentNode;
    if (parent == null) {
        return null;
    }
    if (parent.nodeType === Node.ELEMENT_NODE) {
        return parent;
    }
    return null;
}

function getClassList() {
    var el = this;

    if (this._classList) {
        return this._classList;
    } else {
        var dtlist = new DOMTokenList(
            function _getClassName() {
                return el.className || "";
            },
            function _setClassName(v) {
                el.className = v;
            }
        );
        this._classList = dtlist;
        return dtlist;
    }
}
});
M8.define('pd','utils',function(require, module, exports){
!(function (exports) {
    "use strict";

    /*
        pd will return all the own propertydescriptors of the object

        @param Object obj - object to get pds from.

        @return Object - A hash of key/propertyDescriptors
    */    
    function pd(obj) {
        var keys = Object.getOwnPropertyNames(obj);
        var o = {};
        keys.forEach(function _each(key) {
            var pd = Object.getOwnPropertyDescriptor(obj, key);
            o[key] = pd;
        });
        return o;
    }

    function operateOnThis(method) {
        return function _onThis() {
            var args = [].slice.call(arguments);
            return method.apply(null, [this].concat(args));
        }
    }

    /*
        Will extend native objects with utility methods

        @param Boolean prototypes - flag to indicate whether you want to extend
            prototypes as well
    */
    function extendNatives(prototypes) {
        prototypes === true && (prototypes = ["make", "beget", "extend"]);

        if (!Object.getOwnPropertyDescriptors) {
            Object.defineProperty(Object, "getOwnPropertyDescriptors", {
                value: pd,
                configurable: true
            });
        }
        if (!Object.extend) {
            Object.defineProperty(Object, "extend", {
                value: pd.extend,
                configurable: true
            });
        }
        if (!Object.make) {
            Object.defineProperty(Object, "make", {
                value: pd.make,
                configurable: true
            });
        }
        if (!Object.beget) {
            Object.defineProperty(Object, "beget", {
                value: beget,
                configurable: true
            })
        }
        if (!Object.prototype.beget && prototypes.indexOf("beget") !== -1) {
            Object.defineProperty(Object.prototype, "beget", {
                value: operateOnThis(beget), 
                configurable: true
            });
        }
        if (!Object.prototype.make && prototypes.indexOf("make") !== -1) {
            Object.defineProperty(Object.prototype, "make", {
                value: operateOnThis(make),
                configurable: true
            });
        }
        if (!Object.prototype.extend && prototypes.indexOf("extend") !== -1) {
            Object.defineProperty(Object.prototype, "extend", {
                value: operateOnThis(extend),
                configurable: true
            });
        }
        if (!Object.Name) {
            Object.defineProperty(Object, "Name", {
                value: Name,
                configurable: true
            });
        }
        return pd;    
    }

    /*
        Extend will extend the firat parameter with any other parameters 
        passed in. Only the own property names will be extended into
        the object

        @param Object target - target to be extended
        @arguments Array [target, ...] - the rest of the objects passed
            in will extended into the target

        @return Object - the target
    */
    function extend(target) {
        var objs = Array.prototype.slice.call(arguments, 1);
        objs.forEach(function (obj) {
            var props = Object.getOwnPropertyNames(obj);
            props.forEach(function (key) {
                target[key] = obj[key];
            });
        });
        return target;
    }

    /*
        beget will generate a new object from the proto, any other arguments
        will be passed to proto.constructor

        @param Object proto - the prototype to use for the new object
        @arguments Array [proto, ...] - the rest of the arguments will
            be passed into proto.constructor

        @return Object - the newly created object
    */
    function beget(proto) {
        var o = Object.create(proto);
        var args = Array.prototype.slice.call(arguments, 1);
        proto.constructor && proto.constructor.apply(o, args);
        return o;
    }

    /*
        make will call Object.create with the proto and pd(props)

        @param Object proto - the prototype to inherit from
        @arguments Array [proto, ...] - the rest of the arguments will
            be mixed into the object, i.e. the object will be extend
            with the objects

        @return Object - the new object
    */
    function make (proto) {
        var o = Object.create(proto);
        var args = [].slice.call(arguments, 1);
        args.unshift(o);
        extend.apply(null, args);
        return o;
    }

    /*
        defines a namespace object. This hides a "privates" object on object 
        under the "key" namespace

        @param Object object - object to hide a privates object on
        @param Object namespace - key to hide it under

        @author Gozala : https://gist.github.com/1269991

        @return Object privates
    */
    function defineNamespace(object, namespace) {
        var privates = Object.create(object), 
            base = object.valueOf;

        Object.defineProperty(object, 'valueOf', { 
            value: function valueOf(value) {
                if (value !== namespace || this !== object) {
                    return base.apply(this, arguments);
                } else {
                    return privates;
                }
            }
        });

        return privates;
    }

    /*
        Constructs a Name function, when given an object it will return a
        privates object. 

        @author Gozala : https://gist.github.com/1269991

        @return Function name
    */
    function Name() {
        var namespace = {};

        return function name(object) {
            var privates = object.valueOf(namespace);
            if (privates !== object) {
                return privates;
            } else {
                return defineNamespace(object, namespace);
            }
        };
    }

    var Base = {
        extend: operateOnThis(extend),
        make: operateOnThis(make),
        beget: operateOnThis(beget)
    }

    extend(pd, {
        make: make,
        extend: extend,
        beget: beget,
        extendNatives: extendNatives,
        Name: Name,
        Base: Base
    });

    exports(pd);

})(function (data) {
    if (typeof module !== "undefined") {
        module.exports = data;
    } else {
        window.pd = data;
    }
});
});
M8.define('interfaces/Node','all',function(require, module, exports){
module.exports = {
	contains: {
		value: contains
	},
    interface: window.Element
}

function contains(other) {
    var comparison = this.compareDocumentPosition(other);
    if (comparison === 0 || 
        comparison & Node.DOCUMENT_POSITION_CONTAINED_BY
    ) {
        return true;
    }
    return false;
}
});
M8.define('interfaces/Event','all',function(require, module, exports){
module.exports = {
	constructor: constructor
};

function constructor(type, dict) {
	var e = document.createEvent("Events");
    dict = dict || {};
    dict.bubbles = dict.bubbles || false;
    dict.catchable = dict.catchable || false;
    e.initEvent(type, dict.bubbles, dict.catchable);
    return e;
}
});
M8.define('dataManager','utils',function(require, module, exports){
var uuid = 0,
    domShimString = "__domShim__";

var dataManager = {
    _stores: {},
    getStore: function _getStore(el) {
        var id = el[domShimString ];
        if (id === undefined) {
            return this._createStore(el);
        }
        return this._stores[domShimString + id];
    },
    _createStore: function _createStore(el) {
        var store = {};
        this._stores[domShimString + uuid] = store;
        el[domShimString ] = uuid;
        uuid++;
        return store;
    }
};

module.exports = dataManager;


});
M8.define('interfaces/Event','shims',function(require, module, exports){
var pd = require("utils::pd"),
	Event = require("all::interfaces/Event");

module.exports = pd.extend(Event, {
	constants: {
		CAPTURING_PHASE: 1,
		AT_TARGET: 2,
		BUBBLING_PHASE: 3
	},
	initEvent: {
		value: initEvent
	}
});

function initEvent(type, bubbles, cancelable) {
    this.type = type;
    this.isTrusted = false;
    this.target = null;
    this.bubbles = bubbles;
    this.cancelable = cancelable;
}
});
M8.define('interfaces/DOMException','shims',function(require, module, exports){
module.exports = {
	constants: {
	    INDEX_SIZE_ERR: 1,
	    DOMSTRING_SIZE_ERR: 2, // historical
	    HIERARCHY_REQUEST_ERR: 3,
	    WRONG_DOCUMENT_ERR: 4,
	    INVALID_CHARACTER_ERR: 5,
	    NO_DATA_ALLOWED_ERR: 6, // historical
	    NO_MODIFICATION_ALLOWED_ERR: 7,
	    NOT_FOUND_ERR: 8,
	    NOT_SUPPORTED_ERR: 9,
	    INUSE_ATTRIBUTE_ERR: 10, // historical
	    INVALID_STATE_ERR: 11,
	    SYNTAX_ERR: 12,
	    INVALID_MODIFICATION_ERR: 13,
	    NAMESPACE_ERR: 14,
	    INVALID_ACCESS_ERR: 15,
	    VALIDATION_ERR: 16, // historical
	    TYPE_MISMATCH_ERR: 17,
	    SECURITY_ERR: 18,
	    NETWORK_ERR: 19,
	    ABORT_ERR: 20,
	    URL_MISMATCH_ERR: 21,
	    QUOTA_EXCEEDED_ERR: 22,
	    TIMEOUT_ERR: 23,
	    INVALID_NODE_TYPE_ERR: 24,
	    DATA_CLONE_ERR: 25
	},
	interface: function () { },
	prototype: {}
}
});
M8.define('interfaces/DOMImplementation','shims',function(require, module, exports){
module.exports = {
    createDocumentType: {
	    value: createDocumentType
	}
};

function createDocumentType(qualifiedName, publicId, systemId) {
    var o = {};
    o.name = qualifiedName;
    o.publicId = publicId;
    o.systemId = systemId;
    o.ownerDocument = document;
    o.nodeType = Node.DOCUMENT_TYPE_NODE;
    o.nodeName = qualifiedName;
    return o;
}
});
M8.define('interfaces/Element','shims',function(require, module, exports){
var Element = require("all::interfaces/Element"),
	recursivelyWalk = require("utils::index").recursivelyWalk,
	pd = require("utils::pd");

module.exports = pd.extend(Element, {
	getElementsByClassName: {
		value: getElementsByClassName
	},
	childElementCount: {
		get: getChildElementCount
	},
	firstElementChild: {
		get: getFirstElementChild
	},
	lastElementChild: {
		get: getLastElementChild
	},
	nextElementSibling: {
		get: getNextElementSibling
	},
	previousElementSibling: {
		get: getPreviousElementSibling
	}
});

function getChildElementCount() {
    return this.children.length;
}

function getFirstElementChild() {
    var nodes = this.childNodes;
    for (var i = 0, len = nodes.length; i < len; i++) {
        var node = nodes[i];
        if (node.nodeType === Node.ELEMENT_NODE) {
            return node;
        }
    }
    return null;
}

function getLastElementChild() {
    var nodes = this.childNodes;
    for (var i = nodes.length - 1; i >= 0; i--) {
        var node = nodes[i];
        if (node.nodeType === Node.ELEMENT_NODE) {
            return node;
        }
    }
    return null;
}

function getNextElementSibling() {
    var el = this;
    do {
        var el = el.nextSibling;
        if (el && el.nodeType === Node.ELEMENT_NODE) {
            return el;
        }
    } while (el !== null);

    return null;
}

function getPreviousElementSibling() {
    var el = this;
    do {
        el = el.previousSibling;
        if (el && el.nodeType === Node.ELEMENT_NODE) {
            return el;
        }
    } while (el !== null);

    return null;
}


// TODO: use real algorithm
function getElementsByClassName(clas) {
    var ar = [];
    recursivelyWalk(this.childNodes, function (el) {
        if (el.classList && el.classList.contains(clas)) {
            ar.push(el);
        }
    });
    return ar;
};
});
M8.define('bugs','all',function(require, module, exports){
var utils = require("utils::index");

module.exports = run;

function run() {

// IE9 thinks the argument is not optional
// FF thinks the argument is not optional
// Opera agress that its not optional
(function () {
    var e = document.createElement("div");
    try {
        document.importNode(e);
    } catch (e) {
        var importNode = document.importNode;
        delete document.importNode;
        document.importNode = function _importNode(node, bool) {
            if (bool === undefined) {
                bool = true;
            }
            return importNode.call(this, node, bool);
        }
    }
})();

// Firefox fails on .cloneNode thinking argument is not optional
// Opera agress that its not optional.
(function () {
    var el = document.createElement("p");

    try {
        el.cloneNode();
    } catch (e) {
        [
            Node.prototype,
            Comment.prototype,
            Element.prototype,
            ProcessingInstruction.prototype,
            Document.prototype,
            DocumentType.prototype,
            DocumentFragment.prototype
        ].forEach(fixNodeOnProto);

        utils.HTMLNames.forEach(forAllHTMLInterfaces)
    }

    function forAllHTMLInterfaces(name) {
        window[name] && fixNodeOnProto(window[name].prototype);
    }

    function fixNodeOnProto(proto) {
        var cloneNode = proto.cloneNode;
        delete proto.cloneNode;
        proto.cloneNode = function _cloneNode(bool) {
            if (bool === undefined) {
                bool = true;
            }  
            return cloneNode.call(this, bool);
        };    
    }
})();

// Opera is funny about the "optional" parameter on addEventListener
(function () {
    var count = 0;
    var handler = function () {
        count++;
    }
    document.addEventListener("click", handler);
    var ev = new Event("click");
    document.dispatchEvent(ev);
    if (count === 0) {
        // fix opera
        var oldListener = EventTarget.prototype.addEventListener;
        EventTarget.prototype.addEventListener = function (ev, cb, optional) {
            optional = optional || false;
            return oldListener.call(this, ev, cb, optional);
        };
        // fix removeEventListener aswell
        var oldRemover = EventTarget.prototype.removeEventListener;
        EventTarget.prototype.removeEventListener = function (ev, cb, optional) {
            optional = optional || false;
            return oldRemover.call(this, ev, cb, optional);
        };
        // punch window.
        window.addEventListener = EventTarget.prototype.addEventListener;
        window.removeEventListener = EventTarget.prototype.removeEventListener;
    }
    document.removeEventListener("click", handler);
})();

}
});
M8.define('interfaces/Document','shims',function(require, module, exports){
var throwDOMException = require("utils::index").throwDOMException,
    recursivelyWalk = require("utils::index").recursivelyWalk,
	clone = require("utils::index").clone;

module.exports = {
    adoptNode: {
        value: adoptNode  
    },
    createElementNS: {
        value: createElementNS  
    },
	createEvent: {
		value: createEvent
	},
    doctype: {
        get: getDocType
    },
	importNode: {
		value: importNode	
	},
	interface: function () { },
    prototype: document
};

function createEvent(interface) {
    if (this.createEventObject) {
        return this.createEventObject();
    }
}   

function importNode(node, deep) {
    if (node.nodeType === Node.DOCUMENT_NODE) {
        throwDOMException(DOMException.NOT_SUPPORTED_ERR);
    }
    if (deep === undefined) {
        deep = true;
    }
    return clone(node, this, deep);
}

function getDocType() {
    var docType = this.childNodes[0];
    // TODO: remove assumption that DOCTYPE is the first node
    Object.defineProperty(docType, "nodeType", {
       get: function () { return Node.DOCUMENT_TYPE_NODE; } 
    });
    return docType;
}

function createElementNS(namespace, name) {
    var prefix, localName;

    if (namespace === "") {
        namespace = null;
    }
    // TODO: check the Name production
    // TODO: check the QName production
    if (name.indexOf(":") > -1) {
        var split = name.split(":");
        prefix = split[0];
        localName = split[1];
    } else {
        prefix = null;
        localName = name;
    }
    if (prefix === "" || prefix === "undefined") {
        prefix = null;
    }
    if ((prefix !== null && namespace === null) ||
        (
            prefix === "xml" &&
            namespace !== "http://www.w3.org/XML/1998/namespace"    
        ) ||
        (
            (name === "xmlns" || prefix === "xmlns") &&
            namespace !== "http://www.w3.org/2000/xmlns/"    
        ) ||
        (
            namespace === "http://www.w3.org/2000/xmlns/" &&
            (name !== "xmlns" && prefix !== "xmlns")
        )
    ) {
        throwDOMException(DOMException.NAMESPACE_ERR);
    }
    var el = this.createElement(localName);
    el.namespaceURI = namespace;
    el.prefix = prefix;
    return el;
}

function adopt(node, doc) {
    if (node.nodeType === Node.ELEMENT_NODE) {
        // TODO: base URL change
    }
    if (node.parentNode !== null) {
        node.parentNode.removeChild(node);
    }
    recursivelyWalk([node], function (node) {
        node.ownerDocument = doc;
    });
}

function adoptNode(node) {
    if (node.nodeType === Node.DOCUMENT_NODE) {
        throwDOMException(DOMException.NOT_SUPPORTED_ERR);
    }
    adopt(node, this);
    return node;
}
});
M8.define('interfaces/EventTarget','shims',function(require, module, exports){
var dataManager = require("utils::dataManager"),
    throwDOMException = require("utils::index").throwDOMException,
    push = [].push;

module.exports = {
	addEventListener: {
		value: addEventListener
	},
	dispatchEvent: {
		value: dispatchEvent
	},
	removeEventListener: {
		value: removeEventListener
	},
    interface: window.Element
};

function addEventListener(type, listener, capture) {
    if (listener === null) return;

    var that = this;

    capture = capture || false;

    var store = dataManager.getStore(this);

    var eventsString;
    if (capture) {
        eventsString = "captureEvents";
    } else {
        eventsString = "bubbleEvents";
    }

    if (!store[eventsString]) {
        store[eventsString] = {};
    }

    var events = store[eventsString];

    if (!events[type]) {
        events[type] = {};
        events[type].listeners = [];
    }

    var typeObject = events[type];

    var listenerArray = typeObject.listeners;
    if (listenerArray.indexOf(listener) === -1) {
        listenerArray.push(listener);
    } else {
        return;
    }

    if (this.attachEvent) {
        try {
            this.attachEvent("on" + type, handler);
            
            if (!typeObject.ieHandlers) {
                typeObject.ieHandlers = [];
            }

            var index = listenerArray.length - 1;

            typeObject.ieHandlers[index] = handler;

        } catch (e) {
            /* don't care. can't attach so can't be fired */
        }
    }

    function handler() {
        var ev = document.createEvent("event");
        ev.initEvent(type, true, true);
        that.dispatchEvent(ev);
    }
}

function removeEventListener(type, listener, capture) {
    capture = capture || false;

    var store = dataManager.getStore(this);

    var eventsString;
    if (capture) {
        eventsString = "captureEvents";
    } else {
        eventsString = "bubbleEvents";
    }

    var events = store[eventsString];

    if (!events) return;

    var typeObject = events[type];

    if (!typeObject) return;

    var listenerArray = typeObject.listeners;

    var index = listenerArray.indexOf(listener);
    listenerArray.splice(index, 1);

    if (this.detachEvent) {
        try {
            var ieHandlers = typeObject.ieHandlers;

            var handler = ieHandlers[index];

            this.detachEvent("on" + type, handler);    

            ieHandlers.splice(index, 1);
        } catch (e) {
            /* don't care. Can't detach what hasn't been attached */
        }
        
    }
}

function dispatchEvent(event) {
    if (event._dispatch === true || event._initialized === true) {
        throwDOMException(DOMException.INVALID_STATE_ERR);
    }

    event.isTrusted = false;

    dispatch(this, event);
}

function dispatch(elem, event) {
    var invokeListenerForEvent = invokeListeners.bind(null, event);

    event._dispatch = true;

    event.target = elem;

    if (elem.parentNode) {
        var eventPath = [];
        var parent = elem.parentNode;
        while (parent) {
            eventPath.unshift(parent);
            parent = parent.parentNode;
        }

        event.eventPhase = Event.CAPTURING_PHASE;

        eventPath.forEach(invokeListenerForEvent);

        event.eventPhase = Event.AT_TARGET;

        invokeListenerForEvent(event.target);

        if (event.bubbles) {
            eventPath = eventPath.reverse();
            event.eventPhase = Event.BUBBLING_PHASE;
            eventPath.forEach(invokeListenerForEvent);
        }
    } else {
        invokeListenerForEvent(event.target);
    }

    event._dispatch = false;

    event.eventPhase = Event.AT_TARGET;

    event.currentTarget = null;

    return !event._canceled;
}

function invokeListeners(event, elem) {
    var store = dataManager.getStore(elem);

    event.currentTarget = elem;

    var listeners = [];
    if (event.eventPhase !== Event.CAPTURING_PHASE) {
        var events = store["bubbleEvents"];
        if (events) {
            var typeObject = events[event.type];

            if (typeObject) {
                var listenerArray = typeObject.listeners

                push.apply(listeners, listenerArray);
            }
        }
    } 
    if (event.eventPhase !== Event.BUBBLING_PHASE) {
        var events = store["captureEvents"];
        if (events) {
            var typeObject = events[event.type];

            if (typeObject) {
                var listenerArray = typeObject.listeners

                push.apply(listeners, listenerArray);
            }
        }
    }

    listeners.some(invokeListener);

    function invokeListener(listener) {
        if (event._stopImmediatePropagation) {
            return true;
        }
        // DOM4 ED says currentTarget, DOM4 WD says target
        listener.call(event.currentTarget, event);
    }
}
});
M8.define('interfaces/CustomEvent','all',function(require, module, exports){
module.exports = {
	constructor: constructor,
    interface: window.Event
};

function constructor(type, dict) {
    var e = document.createEvent("CustomEvent");
    dict = dict || {};
    dict.detail = dict.detail || null;
    dict.bubbles = dict.bubbles || false;
    dict.catchable = dict.catchable || false;
    if (e.initCustomEvent) {
        e.initCustomEvent(type, dict.bubbles, dict.catchable, dict.detail);
    } else {
        e.initEvent(type, dict.bubbles, dict.catchable);
        e.detail = dict.detail;
    }
    return e;
}
});
M8.define('interfaces/Node','shims',function(require, module, exports){
var nodeShim = require("all::interfaces/Node"),
    recursivelyWalk = require("utils::index").recursivelyWalk,
	pd = require("utils::pd");

module.exports = pd.extend(nodeShim, {
	constants: {
	    "ELEMENT_NODE": 1,
	    "ATTRIBUTE_NODE": 2,
	    "TEXT_NODE": 3,
	    "CDATA_SECTION_NODE": 4,
	    "ENTITY_REFERENCE_NODE": 5,
	    "ENTITY_NODE": 6,
	    "PROCESSING_INSTRUCTION_NODE": 7,
	    "COMMENT_NODE": 8,
	    "DOCUMENT_NODE": 9,
	    "DOCUMENT_TYPE_NODE": 10,
	    "DOCUMENT_FRAGMENT_NODE": 11,
	    "NOTATION_NODE": 12,
	    "DOCUMENT_POSITION_DISCONNECTED": 0x01,
	    "DOCUMENT_POSITION_PRECEDING": 0x02,
	    "DOCUMENT_POSITION_FOLLOWING": 0x04,
	    "DOCUMENT_POSITION_CONTAINS": 0x08,
	    "DOCUMENT_POSITION_CONTAINED_BY": 0x10,
	    "DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC": 0x20
	},
    contains: {
        value: contains  
    },
    compareDocumentPosition: {
        value: compareDocumentPosition
    },
	isEqualNode: {
		value: isEqualNode
	},
    textContent: {
        get: getTextContent,
        set: setTextContent
    }
});

function contains(other) {
    return recursivelyWalk(this.childNodes, function (node) {
         if (node === other) return true;
    }) || false;
}

function isEqualNode(node) {
    if (node === null) {
        return false;
    }
    if (node.nodeType !== this.nodeType) {
        return false;
    }
    if (node.nodeType === Node.DOCUMENT_TYPE_NODE) {
        if (this.name !== node.name ||
            this.publicId !== node.publicId ||
            this.systemId !== node.systemId 
        ) {
            return false;
        }
    }
    if (node.nodeType === Node.ELEMENT_NODE) {
        if (this.namespaceURI != node.namespaceURI ||
            this.prefix != node.prefix ||
            this.localName != node.localName
        ) {
            return false;
        }
        for (var i = 0, len = this.attributes.length; i < len; i++) {
            var attr = this.attributes[length];
            var nodeAttr = node.getAttributeNS(attr.namespaceURI, attr.localName);
            if (nodeAttr === null || nodeAttr.value !== attr.value) {
                return false;
            }
        }
    }
    if (node.nodeType === Node.PROCESSING_INSTRUCTION_NODE) {
        if (this.target !== node.target || this.data !== node.data) {
            return false;       
        }   
    }
    if (node.nodeType === Node.TEXT_NODE || node.nodeType === Node.COMMENT_NODE) {
        if (this.data !== node.data) {
            return false;
        }
    }
    if (node.childNodes.length !== this.childNodes.length) {
        return false;
    }
    for (var i = 0, len = node.childNodes.length; i < len; i++) {
        var isEqual = node.childNodes[i].isEqualNode(this.childNodes[i]);
        if (isEqual === false) {
            return false;
        }
    }
    return true;
}

function getTextContent() {
    if ('innerText' in this) {
        return this.innerText;
    }
    if ('data' in this && this.appendData) {
        return this.data;
    }
}

function setTextContent(value) {
    if ('innerText' in this) {
        this.innerText = value;
        return;
    }
    if ('data' in this && this.replaceData) {
        this.replaceData(0, this.length, value);
        return;
    }
}

function testNodeForComparePosition(node, other) {
    if (node === other) {
        return true;
    }
}

function compareDocumentPosition(other) {
    function identifyWhichIsFirst(node) {
        if (node === other) {
            return "other";
        } else if (node === reference) {
            return "reference";
        }
    }

    var reference = this,
        referenceTop = this,
        otherTop = other;

    if (this === other) {
        return 0;
    }
    while (referenceTop.parentNode) {
        referenceTop = referenceTop.parentNode;
    }
    while (otherTop.parentNode) {
        otherTop = otherTop.parentNode;
    }

    if (referenceTop !== otherTop) {
        return Node.DOCUMENT_POSITION_DISCONNECTED;
    }

    var children = reference.childNodes;
    var ret = recursivelyWalk(
        children,
        testNodeForComparePosition.bind(null, other)
    );
    if (ret) {
        return Node.DOCUMENT_POSITION_CONTAINED_BY +
            Node.DOCUMENT_POSITION_FOLLOWING;
    }

    var children = other.childNodes;
    var ret = recursivelyWalk(
        children, 
        testNodeForComparePosition.bind(null, reference)
    );
    if (ret) {
        return Node.DOCUMENT_POSITION_CONTAINS +
            Node.DOCUMENT_POSITION_PRECEDING;
    }

    var ret = recursivelyWalk(
        [referenceTop],
        identifyWhichIsFirst
    );
    if (ret === "other") {
        return Node.DOCUMENT_POSITION_PRECEDING;
    } else {
        return Node.DOCUMENT_POSITION_FOLLOWING;
    }
}
});
M8.define('interfaces/index','shims',function(require, module, exports){
module.exports = {
	CustomEvent: require("all::interfaces/CustomEvent"),
	DOMException: require("shims::interfaces/DOMException"),
	DOMImplementation: require("shims::interfaces/DOMImplementation"),
	Element: require("shims::interfaces/Element"),
	Event: require("shims::interfaces/Event"),
	Document: require("shims::interfaces/Document"),
	EventTarget: require("shims::interfaces/EventTarget"),
	Node: require("shims::interfaces/Node")
};
});
M8.define('bugs','shims',function(require, module, exports){
var utils = require("utils::index"),
    documentShim = require("shims::interfaces/Document"),
    nodeShim = require("shims::interfaces/Node"),
    elementShim = require("shims::interfaces/Element"),
	eventTargetShim = require("shims::interfaces/EventTarget");

module.exports = run;

function run() {

// IE8 Document does not inherit EventTarget
(function () {
    if (!document.addEventListener) {
        utils.addShimToInterface(eventTargetShim, document);
    }
})();

// IE8 window.addEventListener does not exist
(function () {
    if (!window.addEventListener) {
        window.addEventListener = document.addEventListener.bind(document);
    }
    if (!window.removeEventListener) {
        window.removeEventListener = document.removeEventListener.bind(document);
    }
    if (!window.dispatchEvent) {
        window.dispatchEvent = document.dispatchEvent.bind(document);
    }
})();


// IE8 hurr durr doctype is null
(function () {
    if (document.doctype === null) {
        Object.defineProperty(document, "doctype", documentShim.doctype);
    }
})();

// IE8 hates you and your f*ing text nodes
// I mean text node and document fragment and document no inherit from node
(function () {
    if (!document.createTextNode().contains) {
        utils.addShimToInterface(nodeShim, Text.prototype, Text);
    }

    if (!document.createDocumentFragment().contains) {
        utils.addShimToInterface(nodeShim, HTMLDocument.prototype, HTMLDocument);
    }

    if (!document.getElementsByClassName) {
        document.getElementsByClassName = elementShim.getElementsByClassName.value;
    }
})();

// IE8 can't write to ownerDocument
(function () {
    var el = document.createElement("div");
    try {
        el.ownerDocument = 42;
    } catch (e) {
        var pd = Object.getOwnPropertyDescriptor(Element.prototype, "ownerDocument");
        var ownerDocument = pd.get;
        Object.defineProperty(Element.prototype, "ownerDocument", {
            get: function () {
                if (this._ownerDocument) {
                    return this._ownerDocument;
                } else {
                    return ownerDocument.call(this);
                }
            },
            set: function (v) {
                this._ownerDocument = v;
            },
            configurable: true
        });
    }
})();

// IE - contains fails if argument is textnode
(function () {
    var txt = document.createTextNode("temp"),
        el = document.createElement("p");

    el.appendChild(txt);

    try {
        el.contains(txt);
    } catch (e) {
        // The contains method fails on text nodes in IE8
        // swap the contains method for our contains method
        Node.prototype.contains = nodeShim.contains.value;
    }
})();

require("all::bugs")();

}
});

// app code - safety wrap


(function(){
M8.define('utils/index','app',function(require, module, exports){
var hasOwnProperty = Object.prototype.hasOwnProperty;


var HTMLNames = [
    "HTMLDocument", "HTMLLinkElement", "HTMLElement", "HTMLHtmlElement", 
    "HTMLDivElement", "HTMLAnchorElement", "HTMLSelectElement", 
    "HTMLOptionElement", "HTMLInputElement", "HTMLHeadElement", 
    "HTMLSpanElement", "XULElement", "HTMLBodyElement", "HTMLTableElement", 
    "HTMLTableCellElement", "HTMLTextAreaElement", "HTMLScriptElement", 
    "HTMLAudioElement", "HTMLMediaElement", "HTMLParagraphElement", 
    "HTMLButtonElement", "HTMLLIElement", "HTMLUListElement", 
    "HTMLFormElement", "HTMLHeadingElement", "HTMLImageElement", 
    "HTMLStyleElement", "HTMLTableRowElement", "HTMLTableSectionElement", 
    "HTMLBRElement"
];

module.exports = {
	addShimToInterface: addShimToInterface,
	throwDOMException: throwDOMException,
	clone: clone,
    recursivelyWalk: recursivelyWalk,
	HTMLNames: HTMLNames
};

function recursivelyWalk(nodes, cb) {
    for (var i = 0, len = nodes.length; i < len; i++) {
        var node = nodes[i];
        var ret = cb(node);
        if (ret) {
            return ret;
        }
        if (node.childNodes && node.childNodes.length) {
            var ret = recursivelyWalk(node.childNodes, cb);
            if (ret) {
                return ret;
            }
        }
    }
}

function throwDOMException(code) {
    var ex = Object.create(DOMException.prototype);
    ex.code = code;
    throw ex;
}

function addShimToInterface(shim, proto, constructor) {
	Object.keys(shim).forEach(function _eachShimProperty(name) {
		if (name === "constants") {
			var constants = shim[name];
			Object.keys(constants).forEach(function _eachConstant(name) {
				if (!hasOwnProperty.call(constructor, name)) {
					constructor[name] = constants[name];	
				}
			});
			return;
		}

		if (!hasOwnProperty.call(proto, name)) {
			var pd = shim[name];
			if (pd.value) {
				pd.writable = false;
			} else {
                
            }
			pd.configurable = true;
            pd.enumerable = false; 
			Object.defineProperty(proto, name, pd);	
		}
	});
}

function clone(node, document, deep) {
    document = document || node.ownerDocument;
    var copy;
    if (node.nodeType === Node.ELEMENT_NODE) {
        var namespace = node.nodeName;
        if (node.prefix) {
            namespace = node.prefix + ":" + namespace;
        }
        copy = document.createElementNS(node.namespaceURI, namespace);
        for (var i = 0, len = node.attributes.length; i < len; i++) {
            var attr = node.attributes[i];
            copy.setAttribute(attr.name, attr.value);
        }
    } else if (node.nodeType === Node.DOCUMENT_NODE) {
        copy = document.implementation.createDocument("", "", null);
    } else if (node.nodeType === Node.DOCUMENT_FRAGMENT_NODE) {
        copy = document.createDocumentFragment();
    } else if (node.nodeType === Node.DOCUMENT_TYPE_NODE) {
        copy = document.implementation.createDocumentType(node.name, node.publicId, node.systemId);
    } else if (node.nodeType === Node.COMMENT_NODE) {
        copy = document.createComment(node.data);
    } else if (node.nodeType === Node.TEXT_NODE) {
        copy = document.createTextNode(node.data);
    } else if (node.nodeType === Node.PROCESSING_INSTRUCTION_NODE) {
        copy = document.createProcessingInstruction(node.target, node.data);
    }
    // TODO: other cloning steps from other specifications
    if (deep) {
        var children = node.childNodes;
        for (var i = 0, len = children.length; i < len; i++) {
            copy.appendChild(children[i].cloneNode(node, document, deep));
        }
    }
    return copy;
}
});
M8.define('main','app',function(require, module, exports){
var shims = require("shims::interfaces"),
	utils = require("utils");

Object.keys(shims).forEach(function _eachShim(name) {
	var shim = shims[name];
	var constructor = window[name];
	if (!constructor) {
		 constructor = window[name] = shim.interface;
	}
	delete shim.interface;
	var proto = constructor.prototype;
	if (shim.prototype) {
		proto = constructor.prototype = shim.prototype;
		delete shim.prototype;
	}

	console.log("adding interface ", name);

	if (shim.hasOwnProperty("constructor")) {
		window[name] = constructor = shim.constructor;
		shim.constructor.prototype = proto;
		delete shim.constructor;
	}

	utils.addShimToInterface(shim, proto, constructor);
});

require("shims::bugs")();
});
})();
})();