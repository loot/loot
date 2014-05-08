var data = {
    lootVersion: '0.6.0',
    masterlist: {
        revision: 'be2c19078',
        date: '',
    },
    masterlistUpdater: 'Enabled',
    globalMessages: [
        {
            type: 'say',
            message: 'Note: <a href="http://forums.bethsoft.com/topic/1498392-rel-loot-thread-7/">Latest LOOT thread</a>.',
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
                    message: 'Note: Requires: <a href="http://www.nexusmods.com/skyrim/mods/60">Dragonborn Compatibility_Patch</a>.',
                },
                {
                    type: 'warn',
                    message: 'Warning: Contains 3 UDR records. Clean with <a href="http://www.nexusmods.com/skyrim/mods/25859">TES5Edit</a>.',
                },
                {
                    type: 'error',
                    message: 'Error: This plugin requires "Dawnguard.esm" to be installed, but it is missing.',
                },
            ],
        },
    ],
};