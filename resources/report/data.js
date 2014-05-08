var data = {
    lootVersion: '0.6.0',
    masterlist: {
        updaterEnabled: true,
        revision: 'be2c19078',
        date: '',
    },
    globalMessages: [
        {
            type: 'say',
            content: 'Note: <a href="http://forums.bethsoft.com/topic/1498392-rel-loot-thread-7/">Latest LOOT thread</a>.',
        },
    ],
    plugins: [
        {
            name: 'Skyrim.esm',
            version: '2.0.0b',
            crc: 'C665FD56',
            tagsAdd: [
                'Delev',
                'Relev',
            ],
            tagsRemove: [
                'Delev',
                'Relev',
            ],
            messages: [
                {
                    type: 'say',
                    content: 'Note: Requires: <a href="http://www.nexusmods.com/skyrim/mods/60">Dragonborn Compatibility_Patch</a>.',
                },
                {
                    type: 'warn',
                    content: 'Warning: Contains 3 UDR records. Clean with <a href="http://www.nexusmods.com/skyrim/mods/25859">TES5Edit</a>.',
                },
                {
                    type: 'error',
                    content: 'Error: This plugin requires "Dawnguard.esm" to be installed, but it is missing.',
                },
            ],
        },
    ],
};