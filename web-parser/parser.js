$(document).ready(() => {
    const input = $("#file-input")[0];

    input.addEventListener("change", Load);
});

function ReadString(view, reader) {
    const length = view.getUint32(reader.offset, true);

    reader.offset += 4;

    const bytes = new Uint8Array(view.buffer, reader.offset, length);

    reader.offset += length;

    return new TextDecoder().decode(bytes);
}

async function Load(event) {
    const file = event.target.files[0];
    const buffer = await file.arrayBuffer();
    const view = new DataView(buffer);

    const engravings = [];

    let offset = 0;

    while (offset < view.byteLength) {
        const reader = { offset };

        const target = ReadString(view, reader);
        const source = ReadString(view, reader);
        const race = ReadString(view, reader);
        const location = ReadString(view, reader);

        const level = view.getUint16(reader.offset, true);

        reader.offset += 2;

        const playtime = view.getFloat32(reader.offset, true);

        reader.offset += 4;

        const date = new Date(Number(view.getBigUint64(reader.offset, true))).toLocaleString();

        reader.offset += 8;

        engravings.push({ target, source, race, location, level, playtime, date });

        offset = reader.offset;
    }

    if (window.table) {
        window.table.destroy();
    }

    window.table = new DataTable("#engravings", {
        data: engravings,
        columns: [
            { data: 'target' },
            { data: 'race' },
            { data: 'level' },
            { data: 'playtime' },
            { data: 'source' },
            { data: 'location' },
            { data: 'date' },
        ],
        order: [[4, 'desc']],
        pageLength: 10,
        responsive: true,
    });
}